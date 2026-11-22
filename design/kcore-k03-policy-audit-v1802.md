# K03 v1802: OOM policy provenance and remaining scope

Date: 2026-11-22. Status: source/history audit; K03 was OPEN
at v1802 and is **CLOSED by policy at v1803**. See
`design/kcore-k01-k04-closure-v1803.md`.

## Current production behavior

The production assembly in
`system/kernel/proc/syscall_dispatch_pdp6.s` explicitly sets
`PROC_OOM_BRK_ENABLED_V1000,1`. This is intentional: DAIMOS
commit `283b54dc` enabled requester-only BRK OOM on 2026-11-07
after v1298 F12/F13 release acceptance. The independent
release record is
`pdp10-doc/KERNEL/OOM-F13-PRODUCTION-ACCEPTANCE-V1298.MD`.
The later v1489 source sweep also recorded the ON setting.
Do not disable it merely because an older checklist says OFF.

`native_sys_brk` calls `vm_space_brk_current`. Only a return
of AC1=-1 **and** AC2=-1 (physical MM_ERR_NOMEM) enters
`native_sys_brk_oom_v1000`, which passes status 1 to the
current owner's `proc_fatal_close_current_v879`. Other BRK
failures return normally. The fatal continuation retains its
private FILE and HUP stack and follows the K04 checked retry
and finalizer protocol. No other process is selected for
termination by this syscall path.

The `proc_swap_victim` selector in `proc_pdp6.s` is a **swap-out**
victim chooser, not an OOM killer. It may evict an eligible
resident process image to backing storage without terminating
the process. Its existence is not evidence that nonrequester
OOM termination is implemented.

## What K03 still means

The open K03 checklist describes safe allocation-free selection
and **termination of a different process** to reclaim memory.
That policy is neither established by requester-only BRK OOM
nor authorized by K04 closure. K04 guarantees owner-safe
quarantine and checked retry, **not** a bound on recovery when
a provider or pinned MM owner refuses forever.

Any future nonrequester OOM policy requires a separate explicit
decision on whether cross-process killing is desirable, which
owners may be targeted, what memory it can actually recover,
and what happens on permanent FILE/MM refusal. Do not silently
reuse the swap selector as a kill selector. For now, preserve
the accepted requester-only behavior. The subsequent v1803
decision explicitly rejects cross-process OOM killing and
closes K03 without any production kernel change.

## Validation boundary

This v1802 stage changes no production kernel instruction,
resident RAM, ABI or OOM policy. The accepted v1298 physical
and dual-host evidence is historical, not a fresh v1802 guest
run. The v1802 testkit source-contract regression checks the
enabled switch, the exact AC1/AC2 dispatch guard, current-owner
fatal path and absence of a foreign kill in the swap selector.
It PASSed with five unsafe mutations rejected.
