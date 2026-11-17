# P1-E v1638: genuine physical FULL relocated RAW restore

Date: 2026-11-17. Base: v1637 accepted PDP-6 48K original six
PACKED child restores with the v1636 low-BCACHE placement fix.
The v1638 change is testkit diagnostics and documentation only.
No KCORE, MM, MRES, process, loader, ABI or BCACHE production
implementation changed.

## Forcing the actual FULL fallback without MM manipulation

The opt-in PURE service first invokes v1384 prefix-only reload.
The original `exec_image_prefix_backing_v1386` callback rejects
a COMPRESSED DXR2 PURE image with its documented -2 UNSUPPORTED
return, causing `proc_pure_mres_service_v1612` to invoke the
bounded full-APR v1391 restore. Both callbacks and the linked
original KCORE/MRES remain completely unchanged.

The v1638 C89 guest preserves the original v1637 six-child
PIPE, private mutable sentinel, EXIT-057 and exact parent
PID/EXITED/status-057 WAIT tests, with the expected seventh RUN
resource refusal and parent no-growth BRK. Child slot 2 uses
the PLAIN `/SYSTEM/INIT` image; child slots 3..7 use the
COMPRESSED+PURE `/SYSTEM/COMP` copy of the identical freshly
linked executable. The original `d6block-v1464` generates the
compressed DXR2 and original `mktsfs-v1461` constructs the
three-member boot volume (54 files), independently validated
by `tsfscheck-v1461`. Image text/BSS and source identifiers
are unchanged apart from the compressed physical representation.

The first plain child can restore and exit by the v1384 prefix
path, freeing its complete physical APR for a subsequent FULL
reconstruction. This makes physically separate source/target
ownership possible without adding temporary kernel allocation
hooks, spoofing MM descriptors or changing the user process ABI.

The complete original source-built physical PDP-6 48K no-AUX
TSFS/GCC/PURE guest prints:

`aaaaaax[B013470/042501]<010000+010000>[A013470/042501]{013470-013470}LwwwwwwZ`

It then reaches normal simulator HALT with six original child
success markers and six exact parent WAIT/EXIT status checks.

## Read-only source-linked physical loader breakpoint

The SAME compiled guest and private Type-551 media copies were
replayed under SIMH. Both B and A original MM snapshots have
SIX unpinned, 007000-octal-word PACKED suffix owners 7..2,
with the long-lived parent APR at `130000..140000`.

The original PURE MRES image and symbol map locate
`exec_image_full_bounded_v1391` at image offset 003224.
Its live physical address is 035252, and the word there
`200000000004` is byte-for-byte source-linked to the original
compiled DXR opcode. SIMH stops at the ACTUAL instruction
`Breakpoint, PC: 035252 (MOVE 0,4)` under scheduler slot zero.

At this exact breakpoint AC2 is physical target 120000 and
AC4 trusted MM capacity 010000, with nonzero backing and stable
u-area arguments. The original MM owner table simultaneously
contains two distinct, unoverlapping, pinned extents for one
compressed process owner (slot 3):

| Original owner | Octal physical interval | MM type | Pins |
|---|---|---|---|
| 3 | `111000..120000` | PACKED_PROCESS | 1 |
| 3 | `120000..130000` | PROCESS | 1 |

The shorter source is the still-authoritative mutable suffix.
The separate complete destination APR belongs to the SAME
process and has already been allocated and pinned BEFORE the
bounded FULL compressed image loader is invoked. It cannot be
an in-place prefix-claim transaction or a resident-only test.

The simulator continues with all remaining child `w` messages,
the parent's final `Z` after the exact six WAIT results and a
normal HALT. The final MM Z snapshot has NO child PROCESS or
PACKED owner 2..7 and retains the unpinned parent APR.

The source-, linked-opcode-, compressed-header- and owner-aware
testkit auditor PASSes. It rejects one forged foreign FULL
target owner and one forged parent WAIT success marker.

## Acceptance boundary

The successful physically relocated v1391 FULL path is now
demonstrated for at least one compressed child (slot 3) with
real concurrently pinned source/target MM ownership and later
functional user execution and cleanup. Earlier v1630 physically
proved the ordinary v1384 fast prefix claim/commit; v1637
physically proved six genuine PACKED restorations. Thus both
successful restoration mechanisms now have physical examples.

The test does NOT individually breakpoint all five compressed
children, does NOT prove interrupted or failed FULL loader
rollback, and does NOT establish whole-system peak/net RAM.
Still OPEN: immutable executable source loss/unmount/exchange
failstop, PI6 contention and failure-atomic MM rollback,
32K/64K general BCACHE regression profiles, and matched
whole-system transient RAM peak and net comparison. PURE remains
disabled by default; strict 32K optimization remains on HOLD.
