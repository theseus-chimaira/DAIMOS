# K05 v1808: restore publication survives a lost success reply

Date: 2026-11-22. Branch:
`kcore-raw-pure-integration-v1804`.
**Scoped PASS, K05 remains OPEN.** This is a correctness fix
to opt-in PDP-6 RAW PURE MRES, not to fixed KCORE or
the default disk-swap service.

## Defect and source-backed reproduction

The original `proc_pure_mres_service_v1612` required BOTH a
zero restore return code and the completed
`!TRANSITION && VM_PDP6_BASE && RESIDENT` publication before
issuing scheduler `READY_ONCE`. The original in-place
`proc_pure_restore_v1384` and bounded
`proc_pure_ram_restore_v1391` commit their MM owner, VM
base, stable-uarea backing and two-bit storage mode before
clearing TRANSITION and returning success. The committed
owner state is authoritative; a lost return value cannot
retroactively revoke it.

The new v1808 testkit native C89 guest executes the actual
PDP-6 original prefix/FULL transactions, original MM rebase
and prefix assembler and real storage-mode instructions.
An isolated caller-boundary wrapper changes ONLY the return
from the *already completed* original transaction from
`MM_OK` to `MM_ERR_BUSY`; it does not forge MM state,
skip loader writes, modify the source process or add
test-only kernel flags. The independent guest assertions
require one exact unpinned PROCESS MM owner, original
immutable backing release, original writable suffix
integrity, resident VM base/mode, cleared TRANSITION,
balanced PI state and immediate `READY_ONCE`.

Before the fix, the unchanged integrated service returned
without readiness after a successful prefix restore:
`__test_exit=000000002004` (octal). This is an
**executed** behavioral failure, not a stale-source
compile error.

## Minimal correction

The service's final readiness test now trusts the existing
committed-state condition independently of the callback
return. A failed transaction cannot satisfy this condition:
the original rollback/owner contract retains TRANSITION
and/or PACKED mode until all fallible MM release operations
complete. The exact existing preflight and original
conservative retry/quarantine branches are unchanged.
The earlier `rc` value still controls the prefix-to-FULL
fallback, and the normal failure path continues to enforce
the existing pressure cooldown.

The source change is one deleted return-code branch, plus
comments. No new data words, process flags, tables, kernel
APIs, or new scheduler state. It is not a blind retry or
an override of an MM refusal.

## Acceptance

* Both original source-linked **prefix and FULL** restores
  PASS despite independent lost-success replies, with
  actual `READY_ONCE` publication.
* A separately compiled original-service mutation restoring
  the former return-code condition FAILS the executed guest
  at the expected `000000002004` exit.
* Original `pure-service-v1612` native DROP,
  prefix/FULL, refusal and unsafe-READY mutant PASS
  after updating its version-specific source anchor.
* Original v1395, v1648, v1649 and v1654 FULL rollback,
  pinned owner, source and commit tests PASS on this
  integrated source. The v1808 combined K03/K04/PURE
  native integration target passes.
* The fresh post-fix real **64K PDP-6 TSFS/PURE**
  physical guest with absent AUXSTORE reached
  `PURE LOADED` and `INIT V1`.
* The additional opt-in v1667 mode3 original-MM
  source-linked native service/full-restore and
  unsafe bitmap-owner retry mutation PASS. This is
  a compatibility check on the shared readiness
  predicate, NOT bitmap production acceptance.

The same-profile KCORE DXR hashes both before and after
the fix are
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
The opt-in PURE MRES generated assembler removes exactly
one `JUMPN`. Its linked DXR host-file size decreases from
16,072 to 16,064 bytes, consistent with one fewer
machine instruction; the removable module is smaller,
not fixed KCORE. These are corresponding two original
64K source-built profiles, not a default-ON release.

## Boundaries and outstanding K05 work

This closes the **lost successful restore response** case
for both original RAW restore transactions. It does not
prove the physically generated lost-result fault, active
PI6 interrupted restore, corrupt/stale source identity,
simultaneous two-owner failure, permanent unknown MM pins,
backend disappearance, or verified full-system continuous
RAM high-water. K05-K08 remain OPEN; K01-K04 remain CLOSED.
No mainline merge, default-mode promotion or obsolete
branch deletion is authorized by this scoped acceptance.
