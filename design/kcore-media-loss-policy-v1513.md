# KCORE v1513: critical media fail-stop, recoverable non-root media

Date: 2026-11-12. Feature policy and source-linked native acceptance,
not optimization, MRES work, or a claim of full physical recovery.

## Authoritative user decision

Confirmed loss of boot root filesystem media or BACKSTORE media is
**crashworthy**. KCORE shall stop the machine rather than attempt
to reconstruct vanished critical storage, terminate suspended
private executive owners, free their original runs, or pretend
that normal finalization or parent WAIT succeeded. A short
distinct root/BACKSTORE console diagnostic is desirable only
when it is small and safe; different HALT operands could identify
the cases without a new resident string or state word.

Loss of a mounted **non-root** filesystem does not itself
stop KCORE. Its I/O must fail in a bounded fashion, the mount
must not be silently discarded or published clean, and other
mounted filesystems must continue to work. Recovery when the
device becomes available is useful. Replacement media must be
revalidated (identity/superblocks/geometry and dirty state)
before any stale BCACHE content can be mistaken for new data.

The critical category wins if a physical medium serves both
root or BACKSTORE and another otherwise optional filesystem.
No new quarantine process state is needed for the fatal case.

## Why a generic read error must not immediately HALT

Current PDP-6 Type-270 and Type-167/236 runtime I/O returns the
same generic negative status for controller PI error or 300-tick
watchdog timeout. A single negative read does not prove permanent
media disappearance or data destruction. Physical v1511 routed
two real Type-236 error completions through production swap-in
rollback, and physical v1512 suppressed two actual completions
so the original PI6 watchdog failed the requests; ordinary I/O
then recovered and independent swapped users executed. Crashing
for *every* negative root/BACKSTORE request would break that
accepted transient-failure behavior.

`storage/fs_backing.s` counts logical D6SET root I/O errors;
`proc/proc_swap_pdp6.s` retains the authoritative DISK owner
after swap-in read error. Neither result currently includes a
distinct confirmed-media-lost status. Runtime KINIT's early
boot `kinit_error18` is disposable and cannot be invoked later.
`proc_exit_halt` is the ordinary last-process exit, and it must
not be confused with a fatal critical-storage stop.

**The terminal-loss decision is settled, but automatic detection
and fatal dispatch are OPEN.** Do not add a heuristic per-owner
retry counter or infer loss from cumulative device error
statistics: both would conflate temporary errors with permanent
loss. Once the physical driver exposes reliable offline/lost
media information, a compact terminal reason and no-return
HALT can be wired at the critical-storage owner boundary. Until
then, the established fail-closed rollback/retained ownership
remains safer than unverified release or spurious kernel crash.

## Native v1513 non-root media return: PASS

Testkit `tests/validation/daimos-nonroot-media-return-v1513.{s,sh}`
links the actual `d6fs_reader_get_block` and `fs_backing_read`
source into a PDP-6 instruction-execution test. The secondary
mount-ID-2 backing callback returns negative for two requests,
and the D6FS reader returns NULL both times without publishing
a clean BCACHE entry. The independent mount-ID-1 root reader
continues to read correctly. When the **same** secondary callback
returns, the same reader succeeds and stores a clean entry.
Another failure does not store a false success; subsequent
availability succeeds again. Both mount IDs remain unchanged.
Three actual linked negative mutants (reversed negative branch,
false non-NULL read-failure result, fabricated backing success)
were rejected. The older v1 D6FS mount-isolation test also
passes after a test-only missing callback stub was repaired;
v674 remount and v9 mount-mutation source tests pass.

Scope: this is a source-linked native first-read failure/retry
and isolation test, not a full guest detachable-media acceptance.
It deliberately models cache misses and proves only that a
failed read does not *create* a new clean cache entry. It does
not demonstrate that already-cached blocks become invalid on
replacement, that dirty blocks can be safely discarded, that
open FILEs or pinned vnodes survive a media swap, or that an
automatic recovery/revalidation procedure has been completed.

The testkit accepted result is revision `8ab7648` and its
`daimos-media-loss-policy-native-v1513.md` report. KCORE runtime
instructions, memory allocation, on-media format and ABI remain
unchanged; only source comments and this policy record are added.

## Revised KCORE completion gates

1. **P0 critical media failure:** obtain a *reliable* physical
   driver/device-offline classification distinct from temporary
   PI/timeout failures; fail-stop on confirmed root/BACKSTORE
   loss, optionally print a tiny distinct diagnostic; physical
   critical-loss acceptance. **No recovery/reconstruction task.**
2. **P1 non-root detachable media:** preserve unrelated mounts
   on failure; develop a checked reattachment/revalidation path
   with BCACHE invalidation, stale dirty data protection and
   precise open-handle/lease rules. Test missing and replaced
   devices separately. Do not silently unmount on sync failure.
3. **P1 mixed provider boundaries:** DEVICE/VOLUME, DTFS,
   BCACHE, LOGSTORE and MEMFS failure integration; avoid
   cascading failure from non-root media to the root/BACKSTORE.
4. **P1 F1 swapless PURE:** immutable backing, MM/APR and mutable
   suffix correctness with measured RAM benefit.
5. **P1 F3 dynamic FSINFO:** 1/4/8/26 mount enumeration, preserving
   other in-progress v1492 work.
6. **P2 only after features:** compression and final KCORE RAM
   optimization with measured net savings.

Accepted v1506 through v1512 critical-media transactional and
watchdog tests are not discarded; they remain valuable for
transient errors and safe pre-crash ownership. VERSION 0.5,
32K HOLD unchanged. MRES unload/reload remains post-1.0.
