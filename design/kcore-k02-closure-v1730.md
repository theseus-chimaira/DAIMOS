# KCORE K02 process-resource cleanup: CLOSED at v1730

Date: 2026-11-20. Verdict: K02 CLOSED. K03 automatic OOM killing,
K04 recovery of permanently unreleaseable resources and the strict
32K permanent-size target remain separate OPEN work.

## Accepted production kernel and scope

Baseline DAIMOS 38e6b4d1 (v1729), testkit 6be1309 (v1730).
The committed production system/kernel tree is byte-for-byte
unchanged after the v1720 low-memory EXEC owner fix 5d6e91c1.
The v1721-v1730 guests modify only disposable source copies.

K02's declared scope in kcore-todo.md is the failure-safe cleanup
of process-owned MM, VM, FILE descriptor, anonymous PIPE and named
FIFO resources across ordinary/fatal EXIT, destructive EXEC and
failed or unpublished RUN. Provider-specific filesystem/device
implementation work is explicitly not included. An unsuccessful
release must retain the unique existing owner and permit checked
retry or controlled fail-stop, without reusing released MM, losing
the u-area, double-freeing an extent, or busy-spinning the CPU.
An unrecoverable owner is not silently discarded.

## Final v1730 acceptance

The versioned daimos-testkit
tests/validation/daimos-k02-final-accept-v1730.sh driver records
the original source commits, one result per test and individual
logs. The native mode was executed as a unit against the unchanged
production kernel: all NINE distinct source-linked PDP-6 tests pass.

| Native gate | Covered boundary |
| --- | --- |
| v1719 | Transactional staged EXEC and backing rollback, unsafe mutants |
| v1720 | Destructive low-memory EXEC, pre/post-VM-NONE failures, two mutants |
| v1722 | Rejected unpublished RUN and duplicate FD maps; TSFS, FILE and PIPE |
| v892 | RUN rollback and exactly-once MM/reaper release |
| v1728 | Repeated session HUP, TTY and fatal FILE-stack ownership, three mutants |
| v970 | Foreign HUP exclusion of private fatal-close continuation |
| v1099 | Typed -4/-5 PIPE/FIFO close and four unsafe mutants |
| v1102 | Bounded reaper backoff and four unsafe mutants |
| v922 | Partial session HUP, committed member skip and retry |

The nine-pass native result manifest is saved under
$TMPDIR/daimos-k02-final-accept-v1730.S4As0T. Its original
console log SHA-256 is
cc0ad212a1e50cf0044cf95b98762e9884a9da7ff31c3de52c1bff3d861054d5.

Four fresh isolated 256K PDP-6 physical guests also ran to full
strict verifier completion and returned exit code zero:

1. v1721: real PROCESS MM free followed by two refused backing
   detaches; original record 037 octal remains owned on both errors.
   Only the third detach succeeds, with no repeat PROCESS MM free.
   Original EXEC owner calls once, slot-zero finalizer twice; FILE
   retry and final parent WAIT accept.
2. v1729: actual session leader TTY and two members, one irrevocably
   finalized VM_NONE member, one real D6FS FD3 in private fatal
   cleanup. Three refused HUPs preserve all live ownership, with
   five correct provider MRES leases and five sync refusals.
   A separate RUN child enters destructive EXEC, publishes VM_NONE,
   suffers a replacement-load failure, fatally exits and is WAITed.
   No owner, TTY, u-area, lease or process accounting errors.
3. v1726: two simultaneously live RUN children retain different
   FILE and PIPE owners. Exactly two physical overlap observations,
   one real PIPE MM_BUSY, one pin/unpin/free and three serialized
   provider calls (two refusals). Both final WAITs complete.
4. v900: named FIFO is still present and reopenable after a real
   pinned-MM final-close refusal, and frees correctly after unpin;
   three close visits, one pin/unpin, one MM_BUSY, zero errors.

Exact freshly recorded runner log SHA-256 values (v1730):

| Guest | Log hash |
| --- | --- |
| v1721 | e9af92f20ad88ed25ec855a42fc491ce02f36b9276b0e24c98f85d74d816bb63 |
| v1729 | 015e8710cf564baf3210a18a9c6fa0271380b3ae9442362872df9dd99884a0b7 |
| v1726 | 3ddb83c089420238552cef8b78ece39fd399792536abf0a8502f03661e70bf2a |
| v900 | a8852636c8893c7863332c14cc7e3f4e188ef6a72b863dc4c8608236cff6f9d6 |

These are log hashes, not hashes of uninstrumented production
images. The instrumented KCORE DXR and full SIMH hashes for prior
accepted runs are in the v1721, v1726 and v1729 design reports.
The v1729 strict verifier separately rejected six unsafe transcript
mutants. The v1730 full-system reruns retain their original strict
counter assertions and cannot pass from HALT alone.

## Reconciliation of historical OPEN notes

The v1729 report erroneously carried forward the post-MM-free
backing-detach scenario as still open. That exact physical guest
already PASSED as v1721, and it passed again in v1730.
The earlier notes are preserved as dated stage reports; this
sign-off supersedes their K02 OPEN verdicts.

Some older reports also call out a missing organic media failure.
K02 explicitly excludes device/provider-specific implementation
work, so that is not a blocker for this process-owner contract.
K02 tests nevertheless inject real errors at the registered D6FS
provider and test actual MM pins, frees, VM retirement, MRES leases,
fatal continuation and session/WAIT behavior.

## Exact RAM and feature boundaries

v1719 added 23 KCORE executable words; v1720 added another 7.
The last matched GCC low-memory disk-root report measured:

| Quantity (36-bit words except address) | v1720 |
| --- | ---: |
| KCORE | 10,461 |
| KCORE plus resident MRES | 17,186 |
| Last permanent address (decimal) | 17,233 |
| New persistent data or per-process words | 0 |

The source tree has had no production kernel changes since that
matched build; v1730 therefore adds zero instruction words,
zero fixed words and zero per-process RAM. This is a code-input
comparison against the previously measured footprint, not a new
32K low-memory link. The independent strict 040000 octal (16,384
decimal) permanent-address gate remains OVER BUDGET, with v1720
free-word accounting -850. K02 closure makes no strict 32K claim.

Separate remaining issues:

- K03: safe automatic OOM victim selection and killing remains DISABLED.
- K04: permanently refused FILE/VM/backing owners, zombies and
  offline retry/recovery policy remain OPEN. Never force-unpin/free
  an unproven owner or clear a suspended private fatal stack.
- Provider-specific real media I/O faults and physical removable-media
  recovery are outside K02's process cleanup scope.
- The 32K residency target and other KCORE feature/optimization
  work remain independently unaccepted.

K01 is already CLOSED. Closing K02 does not imply K03/K04 are ready,
nor does it justify restarting automatic OOM termination.
