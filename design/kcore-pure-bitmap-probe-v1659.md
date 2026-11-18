# KCORE P1-E v1659: bounded RAW bitmap MM probe, no publication

Date: 2026-11-18. Exact base: pushed DAIMOS v1658
`435557a54761d5910f435752c3a03c84afb1bef4`.

## Purpose and release boundary

The v1657 original 48K physical MM capture proved that six real
3,456-word RAW suffixes have 65 private nonzero words apiece. With
v1451 presence masks, each would occupy 96 masks + 65 literals =
161 words. Those 19,770 candidate saved words were not released by
the actual kernel. The v1658 atomic MM transition work corrected
PREFIX and DROP uncertainty but did not implement compressed owners.

The v1659 change includes a separately compiled, explicitly test-only
PDP-6 transaction proving bounded second-owner allocation, bitmap
encoding, stream verification and checked rollback. It is NOT a mode3
compressed process or a physical net saving. A real MM allocator bug
discovered by the independent allocator test requires a minimal
permanent KCORE fix; the codec and C probe remain completely unlinked.
No other CPU architecture is silently admitted.

Current `proc_storage_expect_v30` and `proc_storage_publish_v30`
accept only modes 0=RESIDENT, 1=DISK, 2=RAW PACKED. The permanent
`pdp6_pure_detach_v1609` demands an MM_TYPE_PACKED_PROCESS extent of
exactly the uncompressed suffix length; fast PREFIX assumes the same
geometry. A 161-word compressed descriptor cannot safely be
published as mode2 or freed by that terminal path. Mode3 needs
separately audited terminal, restore and scheduler integration.

## Bounded source-linked transaction

`system/kernel/proc/proc_pure_bitmap_probe_v1659.c` has explicit
`DAIMOS_PURE_BITMAP_PROBE_V1659=1` build admission and is absent from
the permanent KCORE/KINIT/MRES make graph. It requires a noncurrent
SLEEP/STOP process already holding PROC_TRANSITION, valid immutable
PURE backing, existing PACKED mode, absent APR, one exact owner/type/
size RAW suffix and no pins. Source geometry is derived using the
existing PDP-6 PURE sector rule; no layout registry is created.

1. Pin the exact RAW MM extent before reading any mutable word.
2. Count nonzero 36-bit words, compute `ceil(raw/36)+nonzero`, refuse
   sizes greater than or equal to the RAW span (`NO_GAIN=-6`).
3. Allocate exactly that length with the existing no-reclaim MM API,
   alignment one, owner slot and PACKED_PROCESS type. Reject invalid
   destination geometry or any overlap before touching destination.
4. Pin the destination and call the byte-for-byte unchanged native
   v1451 30-TEXT-word encoder, linked as opt-in test source.
5. Verify every reconstructed literal/zero, every bitmap group,
   partial-group padding bits and exactly the complete allocated
   encoded length. The verification has no per-word modulo/division
   and no separate RAW-sized scratch allocation.
6. Unpin the destination, free the destination, unpin the original
   RAW, check the original exact unpinned single owner, report length.
   No VM, MM owner or storage mode publication occurs.

All failures retain the caller's TRANSITION. Allocation failure,
non-beneficial density, poisoned literal and invalid bit mask use
checked cleanup. Rejected destination unpin keeps both owners
pinned; rejected destination free retains the provisional MM extent
and original pinned RAW; rejected source unpin retains RAW pinned.
The impossible overlapping MM allocation is rejected before encoding.
No failed call's presumed side effects authorize blind free/unpin.

## Real original MM allocator bug and corrected admission

The separate genuine PDP-6 `mm_pdp6.s` allocator-leaf execution found
that `mm_alloc_aligned_noreclaim` had `TDNE 012,[-4]`, admitting only
MM types 1..3. The already declared MM_TYPE_PACKED_PROCESS=4 was
therefore rejected as INVAL even with an 883-word free gap. Older
staged packed allocation files also referenced this legitimate type;
the controlled v1384 allocator fixture did not expose the defect.

The core MM type field is THREE bits wide, but valid allocations
are exactly types 1, 2, 3 and 4. The corrected production instruction
sequence uses a 3-bit high-bit mask `TDNE [-10]` and an explicit
upper bound `CAILE 4`, rejecting reserved 5..7, negatives, and any
invalid high-bit value. This adds two instructions to the original
permanent MM source. No added descriptor bits or lookup table.

Actual source-extracted production MM leaves now allocate a 0241-octal
(161 decimal) PACKED_PROCESS extent within the genuine v1657-sized
01563-octal (883 decimal) free gap while a disjoint 06600-octal
(3,456 decimal) RAW extent remains PINNED. The exact HIGH result is
physical base 050122 octal; pin, unpin and MM free restore the sole
unchanged RAW owner. 884-word shortage and descriptor exhaustion
return distinct -1/-3 errors, while invalid types 0,5,7,8,-1
return -4 without mutation. Two independent old and overbroad MM
validator mutants are actually assembled, executed and rejected.
Original existing real-MM v1111 and v1552 test suites still PASS.

The fresh 48K source-built GCC selected TSFS-root + secondary DTC3
PDP-6 guest with corrected MM allocation also PASSes all six process
PIPE/WAIT/EXIT and normal HALT PC000000. Installed fixed KCORE low
end=024255 (v1658:024253), KINIT end=113643 (v1658:113641): both
growths are two words. Fixed KCORE stream SHA256:
0a547e28dd9223e1a26420e4a75f8a6a5ca39836dd62b6a3c3d531ced57fc8f8.
This guest does NOT install or execute the bitmap probe or mode3.

## Source-authentic native evidence

`daimos-testkit/tests/validation/pure-bitmap-probe-v1659/run-v1659.sh`
first executes the original v1658 source-linked FULL/owner matrix.
It then links newly compiled KCC production-probe C and the exact
30-word original v1451 PDP-6 assembler with the v1384 original
source-linked DROP/MM rebase, PROCESS and storage helpers. Only MM
allocator/provider/fault responses are native fixtures, not a fake
encoder. The standalone assembly instructions are compared to v1451.

Executed successful and refusal cases: ordinary 768-word RAW exact
29-word packed length (22 masks + seven literals), NOMEM, target pin
refusal, target unpin refusal, target free refusal, RAW unpin refusal,
altered literal, non-shrinking high-density source, poisoned unused
last-mask bit, fake MM overlap and duplicate owner. A second native
case uses 3,456 RAW words and 65 nonzeros, producing exactly
161 words, with source and owner contents verified afterward.

The separate actually compiled and executed unsafe source mutant
replaces bitmap-stream verification with success; it is rejected at
native `__test_exit=001604`, rather than being counted as an invalid
compile. All positive cases complete with zero native test exit.

Standalone compiled costs (PDP-6 DAS object headers):

| Object | TEXT words | DATA | BSS |
| --- | ---: | ---: | ---: |
| v1451-identical codec | 30 | 0 | 0 |
| v1659 C probe | 586 | 0 | 0 |

These are individually linked test objects. They are NOT installed
MRES image growth, boot KINIT staging, whole-system peak or runtime
RAM reduction. A final compression transaction must share/reduce its
validation where safe rather than blindly paying for a large second
validator in the installed MRES. RAM remains the primary objective,
then execution time. No optimistic net-memory claim is made.

## Required next work

The following are NOT implemented or accepted by v1659: a persistent
compressed owner with mode3, atomic commit that can safely free RAW,
source restoration into a pinned aligned full APR, protected end-to-end
decode without overwriting mutable words, terminal unmount/EXIT/WAIT/
reaper cleanup, all MM pin/free uncertainty and PI6 scheduler stages,
physical negative controls, 48K/64K real compressor on/off comparison,
complete continuous peak RAM (including the currently 586-word probe
and 30-word codec), and final KCORE functional freeze. Strict 32K
acceptance/size-only optimization remain deferred. PURE default OFF.
