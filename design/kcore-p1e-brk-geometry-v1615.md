# P1-E v1615: physical BRK geometry diagnosis before MM changes

Date: 2026-11-16
Baseline: committed v1614 PURE-linker correction and physical 48K TSFS.
State: diagnostic only. Production code, KCORE/MRES footprint and
`DAIMOS_PURE_MRES_ENABLED=0` default are unchanged.

## Typed error and live MM evidence, not kernel instrumentation

The accepted 48K physical no-AUXST1 guest uses the genuine
`dlink --pure` DXR and source-selected PURE MRES. Its original
large BRK request reported only user-level failure. v1615 adds
an **isolated testkit** veneer that executes the original BRK
UUO and captures the existing AC2 internal failure cause,
without extending any production syscall ABI.

The experiment also calls the existing privileged HALT syscall
immediately after the BRK outcome. SIMH then examines the real
physical `mm_extents`, `mm_extent_count`, `mm_arenas`,
`mm_arena_count` and current process slot using symbols from
the linked KCORE map. The testkit octal decoder verifies extent
types, owner IDs, pin counts and physical gaps; it does not
mistake a nonresident PACKED suffix for free RAM. This approach
avoids expensive recurring SIMH CPU instruction breakpoints
and preserves sleeping child owners before EXIT cleanup.

## Accepted 48K placement baseline, BEFORE RAW pressure

A real `010000 -> 012000` BRK succeeds, but the unchanged
historical test's nonresident assertion reports `!R`:
no child was reclaimed. A second physical guest executes
`010000 -> 014000`, reaches the test-only HALT, and yields
the following **actual** MM descriptor geometry in octal:

| Owner | Type | Base | End | Extent length |
|---|---|---|---|---|
| Parent slot 1 | PROCESS | `052000` | `066000` | `014000` |
| Child slot 5 | PROCESS | `066000` | `076000` | `010000` |
| Child slot 4 | PROCESS | `076000` | `106000` | `010000` |
| Child slot 3 | PROCESS | `106000` | `116000` | `010000` |
| Child slot 2 | PROCESS | `116000` | `126000` | `010000` |

The single managed arena is `042562..140000` octal.
Before the parent there is a raw gap `046175..052000`,
only `02000` words of which are APR-aligned contiguous.
The separate high free gap is `126000..137577`, of
length `011577`. There are **zero PACKED extents**
after this successful BRK, as also shown by the unchanged
guest MEMINFO mapped-process accounting. This is an
ordinary physical placement result, not RAW reclaim.

The existing PDP-6 BRK code rounds to `02000`-word APR
quanta. A further growth to `016000` should consume
the last aligned gap before the parent; the following
`020000` request exercises an adjacent-gap refusal,
and potentially physical reclaim and full relocation.
The v1615 stepped diagnostic explicitly tests these
thresholds and captures exact MM ownership before
terminating any child.

The current MM allocator's complete-image fallback needs
space for a *second* full APR extent while the old image
remains owned. The RAW omitted prefix is sector-rounded
at `0200` words and remains smaller than an APR quantum
for this small executable. Summing freed prefixes is
therefore insufficient to prove either an aligned contiguous
gap or a viable full-image allocation; an MM move must
preserve the exact PACKED suffix/owner semantics.

## Stepped pressure: requester termination precedes typed veneer

The retained real 48K guest trace for sequential BRK
`014000`, `016000`, then `020000` is:

    aaaa[B052450/042502]1[1056450/042502]
    2[2060450/042502]<010000+020000>wwww

It has neither the controlled `[A...]H` success marker
nor the test-only `C...F` nonfatal refusal marker. The
simulator later halted at PC zero. At that point the
MM decoder finds current slot zero and only six
DYNAMIC extents: no user PROCESS or PACKED owner
survives. That is a **post-cleanup snapshot** and
cannot be used to reconstruct physical geometry at
the failed BRK.

The actual PDP-6 `native_sys_brk` source distinguishes
an ordinary error from `MM_ERR_NOMEM == -1`. On the
latter it jumps directly to
`proc_fatal_close_current_v879`, terminating the
requester before its test-only BRK veneer can store
AC2 or print a failure marker. The observed child
`wwww` output is consistent with the pipe closing
when its parent disappears. This supports a fatal
OOM hypothesis but does not independently measure
AC2 at the failing instruction. The v1615 replay
now explicitly refuses to call this a live MM
snapshot. No production OOM policy was changed.

To classify the exact failure, the next diagnostic
must stop at the source-mapped fatal BRK branch
*before* process cleanup, inspect AC2 and the
current physical extent table, then determine
whether the RAW suffix was ever published.

## Release gates

The v1615 testkit diagnostics live under
`tests/diagnostics/pure-brk-refusal-v1615/`, with
an octal MM extent parser and a replay option for already
built, versioned physical images. They are NOT promoted
to passing validation solely for reaching HALT or BRK.

Still OPEN: actual published PACKED owner under BRK
pressure, typed physical refusal classification,
successful slot-0 prefix/FULL restoration and PIPE/WAIT,
immutable executable-source hold across inactivity,
real transient MM high-water and positive net RAM gain.
No speculative kernel workaround, extra per-process memory,
compression codec or other CPU backend is introduced.
