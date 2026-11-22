# KCORE K01-K04 closure v1803

Date: 2026-11-22. Status: **K01-K04 CLOSED** for the defined
implementation and release profiles. No new kernel functionality
or RAM is introduced by this decision.

## Final accepted scope

| Item | Decision | Evidence |
| --- | --- | --- |
| K01 | CLOSED v1718 | Permanent MM refusal, exact ownership, fail-closed retry and administrative recovery; see `design/kcore-k01-mm-refusal-v1678.md` |
| K02 | CLOSED v1730 | Failure-safe EXIT, EXEC and RUN resource cleanup; see `design/kcore-k02-closure-v1730.md` |
| K03 | CLOSED v1803 **by explicit policy** | Keep requester-only physical BRK OOM termination and swap-based reclamation; **do not implement cross-process OOM killing**; see `design/kcore-k03-policy-audit-v1802.md` and `pdp10-doc/KERNEL/OOM-F13-PRODUCTION-ACCEPTANCE-V1298.MD` |
| K04 | CLOSED v1801 | Refused-owner retry, private fatal cleanup, orphan adoption/direct FREE and operator recovery; see `design/kcore-k04-closure-v1801.md` |

## K03 final policy (not a newly implemented feature)

The production `native_sys_brk` path already enables
`PROC_OOM_BRK_ENABLED_V1000,1` (DAIMOS `283b54dc`). Only a
physical-memory BRK failure reported as AC1=-1 and AC2=-1
terminates the **requesting** process through its existing
private fatal continuation. Other BRK failure causes retain
their existing return behavior. The accepted F12/F13 v1298
256K PDP-6 evidence includes 64/64 release gates and the
same-source Android/Linux physical/media acceptance. The
v1739/v1743 physical refusal checks further exercised actual
MM exhaustion and refused owners. The v1802 static guard
rejected five unsafe policy changes.

DAIMOS already swaps eligible processes rather than killing
them for resident memory. Cross-process OOM termination would
need a separate victim policy and would not guarantee prompt
recovery when FILE/MM ownership is pinned or permanently
refused. The design choice is **not to add such a killer**.
Do not reinterpret the swap-out selector as a kill selector.
No future K03 implementation work is required under this
policy. A deliberate future change to victim policy would be
a new feature proposal and require separate safety acceptance.

## K01/K02/K04 boundaries retained

K01/K02/K04 keep their existing fail-closed owner contracts:
never free an unproven extent, discard a live private FILE
stack, release a retained resource speculatively, or publish
an uncollectable orphan zombie. K04's bounded retry **rate**
does not imply a bounded recovery time for permanently refused
resources; operator intervention may remain necessary.

K04's bounded acceptance does not claim separately tested
two-owner clock wrap, published pinned PREVM/post-VM session
HUP, or organic controller/media failure. Those remain
documented residual risks, not blockers for the agreed scope.

## Verification and change accounting

The K01 v1718, K02 v1730, K03 F12/F13 v1298 and K04 v1801
acceptances are historical evidence, not new v1803 guest
runs. The K04 v1801 closeout regression independently
reverified both latest PDP-6 post-HALT witness sets and
CLEAN D6FS; the v1802 K03 source contract PASSed.

This v1803 closure is a documentation and backlog-policy
cleanup only: **zero production KCORE text, DATA, BSS, ABI
or permanent-RAM delta**. Historical dated progress entries
remain provenance, not current open tasks. Future work should
focus on remaining K05-K16 features and separate size goals.
