# KCORE v1507: bounded owner-specific swap retry (native acceptance)

Date: 2026-11-12. Scope: KCORE feature completion only; no MRES
unload/reload, userland redesign, OOM victim policy, or optimization pass.

## Source-derived failure and implementation

`proc_swap_in` retains an authoritative DISK record, original BACKSTORE
allocation, stable u-area and private executive continuation on a refused
checked backing release. The slot-0 service requests TERM, but may merely
coalesce a pending event; it cannot safely discard that private context.
Before v1507, allocator pressure used the global 0100-tick
`proc_swap_pressure_ticks` cooldown, while the fatal failure path set no
cooldown. Making all fatal errors use this global cooldown would delay
healthy swapped owners and could select a higher-priority permanently
failing owner repeatedly when the global delay expired.

v1507 instead uses no new process state, table entry or resident RAM word:
the existing stable u-area word 067 (used for finalizer state only during
teardown) holds a retry tag in bit 32 and the low-17-bit clock epoch while
an ordinary RUN owner is still nonresident DISK. On failed `proc_swap_in`,
the slot-0 service sets that owner's timestamp before requesting TERM.
The normal runnable and realtime selectors suppress precisely that
owner for 0100 octal hardware-clock ticks; other swapped owners remain
eligible. At expiry the selector retires the tag. Ordinary successful
swap-in and successful equal-size exchange also retire it.

The retry interval is fixed and nonexponential. Multiple failed owners
have independent timestamps, so two bad swap images cannot simply
alternate indefinitely with no clock progress. The existing global
allocator-pressure cooldown is unchanged. The 17-bit modulo clock
comparison is tested across wrap. No automatic process kill, forced
BACKSTORE release, false READY_ONCE or new STOP/QUARANTINE state is
introduced. Existing PREVM/FINALIZE marker takes precedence; v1507
does not overwrite a marked finalizer word.

## Acceptance completed

Testkit `daimos-backstore-retry-scheduler-v1507.sh` source-links real
PDP-6 normal/realtime runnable selection, real PI6 clock epoch,
slot-0 swap service, real swap-in and checked BACKSTORE verifier/free.
The fixture supplies a real bad bitmap release for slot 2 and a
separate invalid nonresident slot 3, plus a resident bystander.
It observes first failure, independent healthy swapped admission,
normal and realtime suppression, two-failure bystander dispatch,
global allocator-pressure compatibility, exact 0100-tick expiry,
stale-tag retirement, a successful checked repair, and epoch wrap.
Three executable negative mutations (no timestamp, no normal gate,
no RT gate) fail their assertion, not compilation.

The earlier native v1493 false-ready/false-success mutations PASS,
v1505 six forced refusals followed by immediate repair PASS (including
new timestamp-clear assertions), and real-source native v1316/v1317
MM/bitmap rollback PASS with three executable negative mutations.
The v1316 source extraction now includes the current checked verifier
and correct mutation anchor; its unrelated local in-progress changes
remain outside this patch.
The real exchange-publication v788 test now checks exact retirement
in B's valid stable u-area, and the full sector/PURE exchange v854
test passes with its existing synthetic compressed-DXR decode fixture
bound to the current grouped-codec entry; neither test bypasses
the checked exchange transaction.

The isolated full 256K KCC PDP-6 image build completed with rc=0,
using a separate build directory. This proves assembly/link/image
integration but is **not** a booted-guest failure acceptance.

## Open physical and permanent-failure boundaries

No full physical guest with an irreparable bitmap/DEVICE error and
continuously runnable bystanders has yet accepted the new retry
cadence. v1506 physically proved repair followed by owning TERM;
v1507 only adds native scheduler qualification. An indefinitely
lost backing cannot be released safely merely because retries have
been throttled. It stays owned until genuine repair/reconstruction
or a separately verified recovery policy. Do not report a zombie,
free DISK blocks, or claim terminal reclaim in that condition.

Next: physical persistent-failure run (normal and RT selection,
independent healthy swaps, clock cadence, exact original DISK owner,
unchanged private continuation, no starvation). Then mixed-media
DEVICE/VOLUME, DTFS/BCACHE/LOGSTORE/MEMFS fault gates, swapless PURE
and APR restoration, and dynamic FSINFO mount enumeration.
