# v926: Deferred fatal delivery across executive continuations

Status: first TERM-only implementation and isolated event-delivery test; full
owner-return and booted integration acceptance outstanding.
Date: 2026-11-01

## Verified baseline

Native PDP-6 `daimos-fatal-handoff-context-v925`,
`daimos-finish-handoff-context-v925`, and
`daimos-oom-handoff-context-v925` all PASS against current source.
These are extracted production assembly tests, not an actual full-system
CHLD-wakeup injection. Existing remote fatal delivery correctly refuses a
foreign target with WAIT_EVENT, WAIT_CHILD, WAIT_INTR, or a saved executive
PI continuation after its wait class is cleared.

## Problem

The refusal prevents a remote sender from releasing a live private executive
stack. However, TERM is not queued by this refusal. After CHLD wakeup clears
WAIT_CHILD, the saved CTX_PC still identifies an executive return until its
owner is restored. Merely retrying remote fatal cleanup can race that return.

## Required contract

1. Authenticate PID/group delivery exactly as today, before any new state.
2. Distinguish accepted-but-deferred termination from actual completion.
   Do not report success if no durable pending request was recorded.
3. A deferred request must not close FILEs, change VM ownership, release
   the u-area, or discard the saved continuation remotely.
4. Preserve at most the semantics needed for the existing fatal event set:
   INT, TERM, HUP, PIPE. Determine status precedence explicitly before
   reusing any packed pending bit. Never silently alias the ALRM/CHLD bits.
5. The target consumes its own pending fatal request at a safe return boundary
   after restoring its private continuation; do not run its FILE teardown
   inside the PI6 scheduler or with a foreign `file_table` pointer.
6. If FILE SYNC or another provider refuses cleanup, preserve the pending
   request and retain ownership through the existing private retry path.
7. Repeated sends coalesce without duplicate FILE close, HUP delivery,
   VM destroy, or finalization. ZOMB/FREE and reused PID cannot inherit a
   stale request.
8. No persistent kernel words, process-table growth, or ABI extension unless
   all existing packed fields have been audited and a demonstrated need remains.

## Test progression

- Native extracted assembly: CHILD, EVENT and INTR wait classes; saved
  executive handoff with cleared wait bits; repeated fatal send; runnable
  user-return control; genuine FILE failure and retry; exact-once cleanup.
- Booted 256K PDP-6: stall a parent in real WAIT_CHILD; arrange actual CHLD
  wakeup; inject TERM before its executive resume and verify that the parent
  completes its private continuation before teardown. Track pending request,
  target identity, independent bystander progress and ownership witnesses.
- Repeat with held FS provider lease and a blocked foreign waiter. A
  permanently nonreturning provider is an independent P0 recovery problem;
  deferred TERM cannot forcibly unlock that lease.
- Run v921 safety inventory and v918-v925 native regressions after any
  implementation. Automatic OOM killing remains disabled.

## Implementation gate

Before changing `proc_event_apply`, audit all bits in PROC_FDCTL_OFFSET,
PROC_FINALIZE_OFFSET, event pending bitmap, and scheduler state. Then identify
the self-owned safe return path where a pending fatal action can be consumed
without violating the PI6 context restore and stack lifetime invariants.
Do not substitute forced remote cleanup for this handoff.

## First implementation and results

- Booted `daimos-provider-term-live-v926` uses a disposable 256K PDP-6
  image with a real held FS lease, foreign waiter, WAIT_CHILD parent and
  independently runnable bystander. Three deferred TERM deliveries pass,
  lease 4,,0 is unchanged, 72 bystander sleep cycles complete and no errors
  appear. The parent does NOT receive CHLD in this test; full owner-side
  continuation acceptance remains open.
- The existing event-1 bitmap position (LH 000004 in FDCTL) records deferred
  TERM after either the WAIT-class or saved executive CTX_PC guard rejects
  foreign cleanup. Recording coalesces repeated TERM requests, without
  releasing FILE, VM, or u-area ownership. Other fatal events still refuse.
- `mach_user_pdp6.s` checks pending TERM on the owner-side monitor-UUO return
  path after restoring `mach_kernel_sp`, before returning to user mode. It
  clears the bit and enters `proc_fatal_close_current_v879` with TERM status;
  that path already retries provider close on the owner's private stack.
- Native `daimos-deferred-term-v926.sh` PASS: two deferred deliveries across
  CHILD-wait and cleared-wait saved-executive handoff, coalesced bit and no
  remote FILE/VM release, followed by v925 HUP lifecycle assertions.
- Native v925 fatal, finish and OOM handoff regressions PASS unchanged.
- A full packaged `permanent-size` check was attempted but the current
  configured package exceeds the octal 040000 endpoint (`045714`); that is
  not a complete booted acceptance or an isolated size delta.

### Still unproved

- v966 tests a non-root public mount number 17 through the production
  PDP-6 `proc_swap_mount_busy` leaf. The fifth mount-ID bit in the
  existing credential word distinguishes mount 17 from mount 1 for
  both resident and swapped PURE backing, and non-PURE does not pin
  the mount. This is native leaf acceptance, not a complete booted
  non-root unmount syscall test.
- v965 booted a real PURE DXR2 image with guest-initiated filesystem
  operations while other PURE processes remained runnable. Unmount `/`
  and a write to `/SYSTEM/INIT` returned -1; opening the backing for
  writing succeeded, ensuring the write operation was reached. Guest
  completion, deferred TERM and CHLD, VM/u-area exact-once cleanup and
  44 bystander cycles passed. The disposable provider's extra INIT
  close consumes one of the two injected SYNC refusals; the observed
  fatal-owner close counters are 2 attempts, 1 rejection. An unmount
  denial may have independent root-mount reasons, while v961/v962 native
  tests exercise the underlying PURE ownership guards directly.
- v964 booted a 256K PDP-6 image with the guest DXR2 explicitly linked
  `--pure`. The real RUN/TERM workload passes with two injected D6FS
  provider sync refusals, three close attempts, correct one-time VM
  and u-area releases, CHLD=1 and 44 independent scheduler cycles.
  This is PURE executable loader/teardown compatibility, not proof of
  a guest-issued unmount or backing-write refusal during execution.
- v963 validates the source linkage from EXEC path lookup through
  attached mount-root provider identity and the actual VFS unmount/
  write guards. The v961/v962 native mount/write tests pass; full-system
  256K PDP-6 v948 D6FS retry/teardown is again PASS with the new code.
  The boot regression proves compatibility, not that a guest attempted
  a live PURE unmount or executable backing write. That additional
  integration case remains open.
- v961/v962 correct the actual resident PURE mount and write lifetime
  guards. A resident image's executable backing is in its per-slot swap
  record, while a swapped image's record is saved in its u-area. Both
  existing guards read only the u-area, leaving resident PURE users
  unprotected. The two native PDP-6 tests fail before and pass after
  selecting the record by VM base. v958/v959/v960, v804 PURE prefix
  reconstruction and v20 EXEC PURE-bit controls pass. The patch adds
  no static words and does not change the swap-record encoding.
- v961 closes a record-publication validation gap: `proc_swap_attach`
  previously checked vnode provider/kind/range but not that the mounted
  root at the selected slot had the same provider and public mount ID.
  PURE restoration reconstructs provider identity from that root, so a
  mismatch could publish a backing record that later resolves to the
  wrong provider. The check runs before writing either compact record
  or staged mount-extension bit. v961/v960 native checks and the v959
  source contract pass. Full-system fault injection remains open.
- v960 links actual `proc_swap_attach` in a C89 native harness and
  verifies all 26 representable live mount IDs, including the staged
  fifth slot bit before u-area publication. Invalid provider, mount,
  kind, PURE text length and non-SIDL staging are rejected without
  mutation of the record. The test does not prove that the executable
  loader binds the supplied provider to the currently mounted root;
  this contextual check remains a source audit item.
- v959 audits the distinct meanings of the existing one-word swap
  record, guarding attach, detach, swap-out/swap-in commit ordering and
  v958 rollback via testkit source checks. Resident entries encode
  executable backing, mount and PURE text, not a disk span. Disk entries
  encode first block and count, while the prior resident record lives
  in the stable u-area. The guard and v788/v806/v957/v958 regressions
  pass. Arbitrary corruption detection and full fault-injected swap
  publication remain separate acceptance tasks.
- v958 active PDP-6 assembler swap-in rollback fix: after read failure
  or pin failure, a rejected `mm_unpin` or `mm_free` used to fall through
  and clear the per-process TRANSITION lock. The failure paths now jump
  to the existing fatal storage invariant, leaving unresolved MM
  ownership blocked from scheduling. The native fixture tests failed
  unpin, failed free after unpin and failed free after pin failure.
  Earlier v667/v788/v806/v880/v957 controls pass; there is no new
  allocation or process table field.
- v957 checks production C `proc_swap_detach` with resident and disk
  records: only VM base zero permits fallible disk-backing verification.
  A resident process with nonzero VM base takes a nonfailing detach
  bookkeeping path; a disk image with invalid backing returns -1 with
  its record intact. Earlier v880 native `vm_space_destroy` tests
  verify retryability. The proposed post-MM-free fallible-detach error
  is therefore not reachable in the normal resident/disk state split.
  Invalid preexisting swap-record metadata remains outside this proof.
- v956 native ownership test couples production u-area release with
  actual MM extent validation. Wrong owner/type and outstanding pins
  preserve the whole descriptor and extent, a valid release clears
  both base and flag, and repeated release is idempotent. This narrows
  the MM inconsistency search to allocation/publication and VM/swap
  teardown paths rather than the tested u-area release boundary.
- v954 source-contract regression preserves the serial physical-provider
  ownership invariant (LH owner+1, RH busy), distinct waiter event,
  revalidation after wake and one normal unlock path. OOM selection
  excludes a known lease owner and returns no victim for a held lease
  with no encoded owner. Private fatal-close retries keep the owner
  alive and never free its memory or force-unlock a provider. Native
  v866, v915, v916 and v917 provider-lock regressions pass as controls.
  This is a static guard plus native control coverage, not a proof that
  an arbitrary suspended physical I/O can be cancelled safely.
- v953 booted PDP-6 acceptance proves eventual provider recovery after
  *eleven* failed VFS sync attempts (assembler threshold 013 octal),
  eleven owner-stack sleep/retry passes and a capped 01000-octal tick
  delay. The twelfth close succeeds: exactly one victim EXIT, cleared
  FILE descriptor, cleared failure witness and no integrity errors.
  This establishes automatic completion when a provider eventually
  resumes, not safe reclamation of a permanently dead provider.
- v951 confirms the production GETTEARDOWN FILE-stage detail remains
  readable through repeated D6FS provider-origin SYNC errors, with
  three diagnostic captures and preserved VM/u-area ownership. The
  diagnostic reports the actual provider return -1 rather than a
  guessed error. The test HALTs before the fourth provider return.
- v952 fixes a consumer-side FILE count mask in `syscall.h`: the real
  encoder stores up to 16 open descriptors in bits 2..6, whereas the
  old 8-bit count decoder overlapped the first-FD field. Native C89
  tests cover all valid count/FD and six failure codes, with no kernel
  runtime format or memory change.
- v950 full-system diagnostic forces repeated failures inside the D6FS
  SYNC provider itself. A controlled HALT on the fourth attempt shows
  three returned provider errors, four D6FS close entries, zero VM and
  u-area release for the target, one descendant CHLD, 33 independent
  bystander cycles and free provider lease. This proves containment
  through the bounded observation window only. The test does not
  implement provider recovery or establish safe forced cleanup.
- v949 booted PDP-6 test combines failure stages: D6FS provider SYNC
  refuses twice, then succeeds; the first subsequent type-3 u-area MM
  free returns busy, then deferred finalization retries successfully.
  VM teardown executes only once; one CHLD and successful INIT TERM
  reap are observed. The FS lease is free at the test HALT. Faults are
  injected only in the disposable image, not in production code.
- DTFS-specific SYNC/provider tests are intentionally postponed until
  its planned rewrite. Continue D6FS and common kernel P0 acceptance.
- v949 full-system PDP-6 checks combined FILE/provider and MM teardown
  failure atomicity. D6FS provider SYNC fails twice during the target's
  own fatal FILE cleanup; successful third close proceeds to VM teardown.
  Then `mm_free` rejects the slot-5 dynamic u-area once, allowing the
  deferred finalizer to retry. VM destruction remains once, u-area free
  runs twice, CHLD is published once, and INIT reaps the expected TERM
  status. The test runs independent scheduler work; both injected errors
  are in a disposable image and neither is a permanent failure.
- v948 booted provider-origin refusal acceptance: the target opens the
  root D6FS `/SYSTEM/INIT` file and leaves its descriptor open when
  owner-side TERM cleanup starts. A disposable D6FS provider SYNC patch
  returns -1 twice and then invokes its normal zero-return behavior.
  The unchanged VFS and FILE close code reports three attempts and two
  provider errors; VM and u-area release exactly once, CHLD and INIT
  reap succeed, and 45 independent scheduler cycles run. No competing
  held provider lease is used in this fixture. This does not establish
  DTFS error handling or physical-media-origin SYNC failure.
- v947 native post-commit guard: a VM_NONE/FINALIZE owner with deferred
  u-area cleanup rejects repeated HUP as well as INT, TERM and PIPE;
  counters prove no repeat FILE, VM or session teardown. Two slot-0
  finalize retries complete normally, and TERM after the finalized
  process has died is refused. The native PDP-6 regression passes;
  a full-system asynchronous race is still untested.
- v945/v946 directly execute native `proc_event_send` with mixed-state
  members. In v945 slot 2 records deferred TERM and later slot 3 in
  TRANSITION refuses, causing a group error without undoing slot 2.
  In v946 slot 2 refuses first, and an eligible later slot 3 receives
  nothing. These establish ordered partial delivery with stop-on-error.
  Whether this is the intended user-visible group API remains a policy
  question; neither test justifies a kernel behavior change by itself.
- v944 native PDP-6 test includes the real group dispatcher in
  `proc_event_send`, using caller session/domain control and PGRP 7 to
  deliver TERM to a WAIT_CHILD target. Its deferred flag is recorded
  without premature FILE or VM cleanup; v943 ordering still passes.
  The group loop applies targets in slot order and returns on the first
  failed member, so mixed-state partial delivery remains an explicit
  untested policy concern.
- v943 establishes the reverse ordering of v942: INT and HUP refused
  before a deferred TERM may be accepted for the WAIT_CHILD owner.
  Neither refusal sets the pending TERM bit or releases FILE/VM state.
  The later TERM is accepted, duplicates coalesce, and competing
  INT/HUP after TERM remain refused. Native PDP-6 test PASS.
- v942 native PDP-6 test exercises duplicate TERM while the WAIT_CHILD
  owner is already marked for deferred TERM, then delivers INT and HUP
  to that same protected owner. A second TERM coalesces; INT and HUP
  return refusal; no resource teardown occurs in the protected window.
  The subsequent existing fatal lifecycle passes. Other fatal-event
  ordering and process-group scenarios remain unverified.
- v941 bounded refusal diagnostic: a disposable kernel halts at the
  fourth rejected close of a genuine owner FILE entry. With no successful
  close, neither `vm_space_destroy` nor `proc_uarea_release` is entered
  for slot 5. The independent descendant exits and publishes CHLD, and
  25 spinner cycles demonstrate scheduler progress. The FS lease stays
  at 4,,0. This confirms retention while provider close refuses, not
  progress under unbounded rejection or safe forced reclamation.
- v940 replaces the v939 synthetic allocator return with a real MM
  extent-table pin: the first matching slot-5 dynamic u-area free sets
  the pin bit and executes production `mm_free_busy`; the retry clears
  that test pin and passes through normal validation/deletion. Full
  256K PDP-6 held-provider/CHLD/TERM acceptance PASS, MM calls 2,
  busy 1, u-area release calls 2, VM destroy 1, no finalizer failure.
  The pin is injected by a disposable fixture, not a concurrent owner.
- v939 booted PDP-6 regression faults the actual `mm_free` interface for
  a type-3 kernel-dynamic extent owned by slot 5 (owner 01005). The first
  call returns busy without changing the MM extent; the second executes
  production `mm_free`. Two allocator and u-area calls, one VM destroy,
  three FILE closes with two earlier refusals, one CHLD and full INIT
  TERM reap: PASS with held FS owner/waiter. A naturally arising internal
  MM allocation-table busy condition has not been exercised.
- v938 booted 256K PDP-6 deliberately fails the first call to
  `proc_uarea_release` for the deferred-TERM owner after VM destruction.
  The existing deferred finalizer retries from slot 0: two u-area
  release entries, only one VM destroy, three FILE entry closes (two
  earlier refusals), one CHLD, correct WAIT/reap and intact held FS lease.
  Test PASS; this is an entry-level failure, not an actual `mm_free`
  rejection. Provider-origin sync and permanent failure remain open.
- v937 stage witness extends booted v936 with slot-5 counters at actual
  `vm_space_destroy` and `proc_uarea_release` entry points. Both execute
  exactly once after two injected FILE entry refusals, with a third close
  attempt succeeding. No descriptor integrity or finalizer error; CHLD,
  owner return, held provider lease and INIT TERM status checks all PASS.
  This establishes one successful post-close teardown, not idempotence
  across separate MM release failures or a real provider VFS_SYNC refusal.
- v936 extends v935 to a real descriptor: the target duplicates its
  output FILE before WAIT_CHILD; injection at `file_close_entry_v865`
  rejects one selected live descriptor twice, preserving its nonzero
  entry, before permitting the unchanged close path. Full-system PDP-6
  run PASS: targeted calls 3, refusals 2, descriptor witness errors 0,
  finalizer failures 0, CHLD 1 and retained provider lease 4,,0.
  Injection occurs before VFS_SYNC; actual provider-origin failures
  and independently counted VM/u-area teardown remain open.
- v936 booted full-system FD-specific retry PASS: the owner (slot 5) has
  its actual stdout descriptor (FD1) pass through `file_close_entry_v865`.
  A disposable-kernel test hook at `file_close_vfs` injects two VFS_SYNC
  failures for that exact descriptor, then executes the original sync.
  The entry remains live across failed syncs; third attempt succeeds;
  finalizer=1, failures=0, CHLD=1, peer cycles=36, held provider lease
  remains 4,,0, and INIT reaps TERM correctly. This tests descriptor-level
  refusal propagation rather than v935's `file_close_all` entry shortcut;
  the provider refusal is injected, not an actual device I/O error.
- v935 booted real scheduler with held FS owner/waiter and deferred TERM:
  a disposable-kernel `file_close_all` entry hook refuses exactly the
  first two calls for the target owner and then executes unchanged close.
  Three calls, two refusals, zero finalizer failures, CHLD=1, complete
  INIT reap, unchanged lease 4,,0, 36 bystander cycles: PASS. This tests
  owner-stack backoff/retry but not a real open descriptor or provider
  rejection, nor teardown multiplicity in every failure mode.
- v934 held-provider full-system control PASS: a real FS lease owner
  (slot 3) and waiter (slot 4) remain suspended across deferred TERM to
  WAIT_CHILD parent (slot 5), descendant EXIT (slot 6), CHLD wakeup,
  owner-side TERM consumption and INIT WAIT/reap. Owner, waiter and final
  lease are all 4,,0; no finalizer failure and 36 independent bystander
  cycles. The initial fixture ran TERM too early for the waiter; adjusted
  timing establishes both wait conditions. This verifies safe preservation
  of the held provider, not forced recovery or booted FILE-close retries.
- v933 resolves the earlier apparent TERM status mismatch: WAIT returns
  `SYS_WAIT_STATUS(SYS_WAIT_EXITED, 0400001U)`, not just `0400001U`.
  The corrected booted guest prints completion `P` and HALTs with child
  finalization, CHLD notification and parent reap all observed. The legacy
  test parser required `SSSP` immediately adjacent; the valid trace is
  `...SSSZSSP`. Rerunning the corrected assertion on the preserved image
  transcript PASSes. Provider-held CHLD/retry completion remains unproven.
- v932 fixes the demonstrated owner-vs-foreign classification error:
  `proc_exit_finish` must clear `proc_current_slot` before idle-stack
  teardown, but the assembler finalizer previously compared the zeroed
  current slot with the owner and enforced foreign CTX_PC restrictions.
  A dedicated owner entry identifies self-EXIT without resident state;
  foreign calls retain all guards. Booted stage witnesses now observe
  descendant `proc_finish_slot` success and CHLD notification. The
  subsequent INIT status assertion fails (`!A`) and needs examination;
  this is not full end-to-end acceptance.
- Booted v931 confirmed the precise first failure *function*: slot 6
  reaches `proc_finish_slot`, which branches once to `proc_finish_fail`;
  no CHLD notification is published. A possible cause requiring direct
  verification is that `proc_exit_finish` resets `proc_current_slot` to
  zero before `proc_finish_slot` tests whether the exiting slot is the
  current owner. Do not weaken noncurrent teardown protection without
  proving an owner-safe alternative.
- Booted v930 adds a descendant-slot-6 `proc_exit_current` witness. Its
  count is 1 while `proc_notify_parent(5)` remains 0, with the owner-return
  witness at 2. Thus the guest reaches actual EXIT but the notification
  boundary is still not observed; inspect finish-stage outcomes before
  assigning blame to CHLD handling. No FS lease is artificially held.
- v929 source-level stage instrumentation distinguishes parent return-gate
  entries from actual CHLD delivery. Its accelerated no-provider test printed
  the descendant's pre-EXIT `Z` marker but sampled zero `proc_notify_parent(5)`
  calls and did not complete the INIT wait. This locates the next probe at
  descendant EXIT/finish and parent notification, not provider cancellation.
  The completed TERM handoff is not accepted.
- Corrected v929 guest confirms descendant reached pre-EXIT `Z`, but no
  `proc_notify_parent(5)` stage witness was observed (owner-return gate 2).
  The next control must distinguish ENTER EXIT, FILE closure, slot finishing,
  and actual CHLD publish. DCS0 and GE0 simulator port binds also failed
  because their configured ports were occupied; avoid overlapping ports
  for a clean repeat. No production defect is yet localized.
- First v929 stage-witness run: CHLD notification counter 0, owner-side
  return-gate counter 2, and no INIT completion. This bounds the observed
  failure before confirmed CHLD notification, but the descendant EXIT has
  not been established. The test now preserves AC2 across its injected
  owner witness and writes guest marker `Z` after the descendant's sleep
  returns. This corrected fixture has not yet been rerun.
- The v928 no-stalled-provider control likewise reaches the three deferred
  TERM guards (one WAIT_CHILD) but does not reach INIT's completion marker.
  No FS gateway owner/waiter is active and the lease is zero. This rules out
  the deliberately held provider as a necessary precondition, but does not
  yet identify whether the failure is in CHLD scheduling, TERM consumption,
  teardown, or the test's WAIT/status assumptions. Add stage witnesses.
- An initial booted CHLD/TERM diagnostic (`daimos-child-term-handoff-v927`)
  deliberately retains the blocked FS owner and waiter while arranging an
  actual child EXIT. It does not pass: INIT's final completion hook is not
  reached (`final_lease` zero), despite three deferred TERM guards and 88
  independent bystander cycles. A separate non-stalled-provider control and
  stage-specific witnesses are necessary to distinguish a teardown wait from
  a regression in deferred TERM. This is not release acceptance.
- Native `daimos-deferred-owner-return-v926.sh` now executes the production
  helper and the existing private fatal-close/retry path. PASS: TERM is
  consumed once, two FILE refusals cause two sleeps, then successful close
  completes with TERM exit status. A second helper invocation does not
  repeat cleanup. The no-TERM return preserves AC2 and AC3 explicitly, with
  a native test asserting both values. This is extracted assembly, not a
  booted process.
- Actual booted CHLD wakeup-to-CTX restore TERM injection, exact-once
  completion and independent bystander progress.
- Determine and test precedence of deferred TERM against other pending fatal
  events, STOP/CONT and nonfatal event visibility. The current increment
  deliberately handles TERM only.
- Repeat full linked image and runtime acceptance with a size-valid profile.
