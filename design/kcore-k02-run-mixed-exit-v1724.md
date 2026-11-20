# K02 physical mixed PIPE MM and filesystem-sync EXIT refusal v1724

Date: 2026-11-20. Status: full-system PDP-6 acceptance PASS; K02 OPEN.

## Objective

Prove that ordinary EXIT of one real RUN child preserves *two separate*
resource owners when their release operations fail, and that retry of
one owner does not prematurely free or replay the other owner's state.
The two owners are an anonymous PIPE MM extent and a regular FILE whose
VFS sync can independently refuse close. Both use the production FILE
close-all and process EXIT logic, not fabricated C cleanup calls.

## Reproducible guest

The versioned testkit files are:

- `tests/validation/daimos-k02-run-mixed-exit-v1724.sh`
- `tests/validation/daimos-k02-run-mixed-exit-v1724.py`

The Python generator derives its physical guest from the committed
v1723 test script and C89 user program, checks exact unique source
anchors, and creates temporary, versioned generated sources. It adds
an open of the actual `/SYSTEM/INIT` D6FS regular file. Parent FD7
is copied by RUN to child FD6. Parent releases its copy, together with
both parent-owned data-PIPE references, before sending a byte over a
separate control PIPE. The child cannot attempt EXIT prematurely.

The first EXIT scans child FILEs in FD order. Its data-PIPE reader FD3
closes successfully; last PIPE writer FD4 cannot be released because
the selected real physical PIPE extent was pinned in the copied kernel
using the production `mm_pin`. The real `mm_free` returns -5 and retains
FD4 and the `readers=0, writers=1` reference. Later in the same FILE
pass, the independently owned regular FD6 is refused by a versioned
test-only `vfs_sync` hook, returning -5 without modifying FD6. This
leaves exactly two child FILEs. Ordinary `dsys_exit` reports failure,
and the original child process continues executing its user image.

On the second EXIT the isolated PIPE hook runs real `mm_unpin` and the
original `mm_free` commits exactly once. The second VFS sync call still
refuses, leaving only FD6. The child then attempts a third EXIT; the
VFS hook delegates to the **original provider-dispatch code** and
the descriptor closes. Parent `WAIT` observes an exited child with
status octal 065; the parent's `SYS_MEMINFO` confirms process-slot,
process-word and FILE-slot occupancy returned to baseline. The test
requires an exact `P` line and successful user-requested HALT.

## Instrumentation limits

Test-only patches are applied to copied `ipc/pipe.s` and `fs/vfs.s`.
The PIPE branch uses the same physical pin/busy/unpin mechanism as
v1723. The VFS branch matches the current child's **exact FD6 vnode**,
not merely a provider number or a class of files. Other VFS operations
are unaffected, including the parent's successful close of FD7.

The filesystem refusal is a return-code injection **at the VFS-sync
boundary**. It is not an independently generated error from the D6FS
backend, tape, storage hardware, or an on-disk defect. The final sync
executes unchanged production VFS/provider dispatch. This limitation
matters for K02 acceptance; do not represent it as device-failure
coverage. No production kernel source or permanent data was modified.

## Measured booted 256K PDP-6 result

| Physical witness | Observed |
| --- | ---: |
| PIPE object base (decimal) | 21,589 |
| RUN child slot | 2 |
| PIPE final-close calls | 2 |
| Real MM pin / unpin | 1 / 1 |
| Actual PIPE MM_ERR_BUSY refusals | 1 |
| Accepted final PIPE MM frees | 1 |
| Matching child filesystem `vfs_sync` calls | 3 |
| Injected filesystem sync refusals | 2 |
| Ownership instrumentation errors | 0 |
| Child FILEs after first/second refused EXIT | 2 / 1 |
| Final parent WAIT / MEMINFO / HALT | PASS |

SHA-256 of copied, instrumented KCORE DXR:
`09a6959d3a9c7da824a8dc99ad1898054e3fc59c53775788adbc73149962fa54`.
SHA-256 of the physical SIMH transcript:
`6fb95cd9d923ea6f5867288db75a4c13919087bc781df378862bb041381fe80a`.

The generated guest was compiled with the PDP-10 C89 compiler and
linked successfully. The executable physical test returned zero with
all exact-count assertions passing. The v1722 native mixed-FD test
with two unsafe source mutants also passed on the current production
checkout, and the previously accepted v1723 physical guest verifier
was rerun successfully.

## Footprint and open K02 gates

Production KCORE text delta: **0 words**. Permanent-data delta:
**0 words**. No per-process fields, no new owner table and no runtime
allocation are introduced. `pdp10-doc` is intentionally untouched
until K02 completion.

Still open are organic provider/media failures, permanently refusing
owners, concurrent HUP/EXIT/fatal EXEC with PIPE/FIFO and provider
interleavings, and bounded reaper recovery (K04). K03 OOM victim
termination remains disabled until its safety prerequisites are met.
