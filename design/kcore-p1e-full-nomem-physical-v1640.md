# P1-E v1640: actual physical FULL NOMEM rollback and retry

Date: 2026-11-17. Base: v1639 accepted matched physical
six-PACKED 48K RAM gain, and v1638 source-backed successful FULL
relocated restoration. v1640 changes ONLY daimos-testkit
diagnostics and documentation. No production KCORE, MM, VFS,
process, loader, PURE MRES or ABI implementation changes.

## Natural FULL allocation refusal, no fault-injected kernel

The v1638 guest was source-derived again, with its compressed
and plain child order reversed. Slot2 launches the original
COMPRESSED+PURE DXR2 executable from `/SYSTEM/COMP`; slot3
uses the original PLAIN+PURE image `/SYSTEM/INIT`; slots 4..7
are compressed. The two DXRs are the same C89 program with
different immutable physical payload representations, built
by the original dlink/d6block and three-member TSFS toolchain.

All six children sleep on their inherited PIPE, parent attempts
a seventh RUN which is honestly refused, and six 007000-word
mutable suffixes are PACKED before parent B/A source-linked
checkpoints. At this point total physical free words are too
few for an additional complete 010000-word aligned APR. The
original v1384 in-place PURE loader returns -2 UNSUPPORTED for
a COMPRESSED DXR2 source. The original v1612 service calls the
bounded v1391 FULL loader, which pins the writable source and
asks original `mm_alloc_aligned_noreclaim` for a new full APR.

The original complete 48K PDP-6 no-AUX/TSFS/PURE guest console:

`aaaaaax[B013470/042501]<010000+010000>[A013470/042501]{013470-013470}LwwwwwwZ`

It ends in normal simulator HALT after all six child PIPE/data/
EXIT-057 and six exact parent PID/EXITED/status-057 WAIT checks.

## Original read-only breakpoint and MM owner evidence

The SAME compiled guest and private source-matched Type-551
tape copies were replayed. Linked MRES offset 002307 is the
original PUSHJ to `mm_alloc_aligned_noreclaim`; its physically
installed instruction immediately after the call is PC 034336.
At this actual breakpoint AC1 = 777777777777 (MM_ERR_NOMEM -1).
The source is still the single exact slot2 PACKED owner at
physical 121000..130000 octal, PINNED=1. Every other PACKED
owner is unpinned and unchanged, no second PROCESS destination
was allocated or published, and slot-zero scheduler is active.

The next original v1612 service return PC 032461 was linked
from the v1391 invocation at MRES offset 000432. AC1 STILL
equals -1 and the original physical MM table has exactly the
SAME six PACKED extents, with ALL PINNED=0. No child address
or extent was moved, no destination survived allocation failure,
and no invalid resident mode was published.

After the plain slot3 child is restored and releases its APR,
the SAME originally refused compressed slot2 is retried. At
the original source-authenticated bounded FULL loader PC 035252,
AC2=110000 is the new destination and AC4=010000 is the trusted
allocated APR length. The original MM table simultaneously has:

| Slot | Original physical extent (octal) | MM type | Pin count |
|---|---|---|---:|
| 2 | `121000..130000` | PACKED_PROCESS suffix | 1 |
| 2 | `110000..120000` | PROCESS destination | 1 |

These two extents are nonoverlapping and independently owned by
the SAME slot. The original loader then completes, the six
children read PIPE data and check both private mutable sentinels,
exit status 057, and the parent performs six exact WAIT checks.
At final source-linked Z, there are no remaining slot2..7
PACKED or PROCESS MM owners. The parent remains resident.

The original physical source/opcode/descriptor/console auditor
PASSes and rejects two independently forged traces: an extra
slot2 RAW source pin after supposed rollback, and a fabricated
successful allocator result at the first NOMEM breakpoint.

## Acceptance boundary

This physically closes **natural FULL APR allocation refusal**:
genuine NOMEM, correct source unpin and intact RAW ownership,
retry when memory becomes available, later successful bounded
FULL reconstruction, user execution and complete WAIT/EXIT/MM
cleanup. The v1638 independent successful FULL and v1630
in-place FAST still pass, and the v1639 1314-word sampled net
advantage remains separately valid.

The original v1395 native testkit already exercises several
injected allocator/MM/loader failure paths, but v1640 does NOT
equate their host-local tests with a physical failed VFS read.
Still OPEN: physically injected VFS/source-loss/unmount failure
during a FULL transaction; PI6/MM error interleavings and
uncertain failure rollback; source lifetime/immutability;
32K/64K low-BCACHE regressions; and complete matched physical
transient peak RAM. PURE remains OFF by default; strict 32K
whole-KCORE size optimization HOLD pending complete acceptance.
