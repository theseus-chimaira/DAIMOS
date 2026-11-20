# K02 physical RUN/PIPE and D6FS provider-lease refusal v1725

Date: 2026-11-20. Status: full-system 256K PDP-6 PASS; K02 OPEN.

## Goal

The v1724 RUN/PIPE/regular-FD guest retained independent PIPE MM and
filesystem FILE owners, but it returned the filesystem error at VFS
before entering the serialized provider MRES. v1725 moves the two
injected sync failures inside the actual D6FS provider SYNC entry and
checks the real serialized filesystem lease on every invocation.
The production VFS, FILE, RUN, PIPE, MM, scheduling, MRES gateway and
WAIT logic are unchanged.

Native D6FS provider SYNC normally returns success because D6FS writes
are synchronous. These two failures are deliberately injected inside
the copied provider entry: the test does not claim to reproduce an
organic disk/media error or corrupt persistent storage.

## Reproducible full-system guest

Versioned testkit files:

- tests/validation/daimos-k02-run-provider-lease-v1725.sh
- tests/validation/daimos-k02-run-provider-lease-v1725.py

The generator reuses the v1724 C89 guest and v1723 real PIPE MM pin
instrumentation. The parent creates two PIPEs and opens the real D6FS
file /SYSTEM/INIT; RUN passes data PIPE reader/writer, control reader,
and regular FILE FD6 to the child. The control PIPE delays child EXIT
until the parent releases all its data-PIPE and regular-file copies.

The first ordinary child EXIT closes the PIPE reader but retains the
final writer FD4 when actual pinned MM free returns MM_ERR_BUSY (-5).
The D6FS provider also refuses the first SYNC of FD6, retaining that
separate FILE owner. The child observes two live FDs. During second
EXIT, real MM unpin/free succeeds once, while D6FS refuses FD6 again;
the child observes only one live FD. The third EXIT reaches the original
D6FS success path. The parent receives WAIT exit status 065 and its
MEMINFO process-slot, process-word and FILE-slot counts return to baseline.

On each of the three targeted calls, the provider hook verifies
fs_provider_ready: left half contains RUN child slot+1, right half is
zero (leased). This confirms that registered FS_MRES_OP_SYNC dispatch
acquired the shared provider lease before invoking D6FS. The normal
fs_provider_reg_call and fs_provider_unlock paths are not modified.
Successful repeated provider entry and final WAIT also exercise lease
release and reentry after each returned error.

The four provider counters live in the copied *resident* fs_mres.s,
while the hook executes inside movable D6FS MRES. The initial test
version allocated them in the MRES, giving module-relative addresses
not present in kcore.map; its diagnostic retrieval failed before
simulation. The corrected version uses fixed KCORE addresses and
completed a full simulator run. No permanent production words are used.

## Physical acceptance counters

| Witness | Result |
| --- | ---: |
| PIPE object base (decimal) | 21,594 |
| Real RUN child slot | 2 |
| PIPE final-close calls | 2 |
| Actual MM pin/unpin | 1 / 1 |
| Actual MM_ERR_BUSY returns | 1 |
| Final successful PIPE MM frees | 1 |
| D6FS provider SYNC calls | 3 |
| Injected D6FS provider refusals | 2 |
| Correct lease observations | 3 |
| Wrong lease observations | 0 |
| Other PIPE/FILE ownership errors | 0 |
| FILE occupancy after first/second rejected EXIT | 2 / 1 |
| Parent WAIT, MEMINFO and HALT | PASS |

Test-only instrumented KCORE DXR SHA-256:
200572fdc36536969f528b2d51bd2ad07e7036e7a310ce085a07e51044861eb9

Physical SIMH transcript SHA-256:
e68cac16bab19ba6a9bf436fbc4068d3403b112cfd2bf0e62c4893706712754c

The corrected standalone test returned success with all exact counter
assertions, the real child slot, the parent-completion P line, and the
expected user-requested HALT. No verdict relies on HALT alone.

## Remaining work

Production KCORE text delta: 0 words. Permanent data delta: 0 words.
No per-process state, owner table, runtime allocation, ABI change,
production fix, or workaround.

This accepts provider-entry rejection under the real serialized MRES
lease but not genuine media I/O errors, permanently refused providers,
or every concurrent HUP/EXIT/fatal-EXEC owner interleaving. Those are
open K02/K04 tasks. The pdp10-doc checkout stays unchanged until K02
closes. K03 automatic OOM termination remains disabled.
