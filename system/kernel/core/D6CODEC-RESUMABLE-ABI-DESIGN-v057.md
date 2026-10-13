# D6 codec v057 - fixed low-core resumable decoder design

Status: design and validation gate; NOT integrated into production.

## Non-negotiable compatibility

The production decoder must be the single implementation installed by all
five Stage-1 loaders and linked at 000060 by KCORE. Its interface is the
existing D6LZ36 fixed-core interface, NOT the v053/v056 complete-buffer API:

- AC12: current destination word address.
- AC13: output words remaining.
- AC3: current source address.
- AC4: input words available from AC3.
- AC14: original output address, used for history validation.
- AC10: token-control state; AC11: tokens remaining (zero at group boundary).
- AC0: 0 complete, +1 NEED_INPUT, -1 invalid stream.
- AC17: caller pushdown pointer, unchanged upon return.

No additional permanent BSS, no dynamic allocation, no larger VFS stack
window, and no per-call stack frame in the decoder. Preserve AC3, AC4,
AC10..AC14 across NEED_INPUT, with AC3 and AC4 identifying any unconsumed
group prefix. Scratch AC0..AC7 only; AC15 and AC16 are preserved by the core.
The old frontend already saves AC10..AC15, and uses a 64-word input window;
its frame must not grow. Stage-1 already stages a complete compressed image.

## Atomic-group streaming

The v053 wire format puts match descriptors before literals in each group.
Consequently a single word of input is not necessarily enough to execute
its first token. Avoid a second descriptor buffer by requiring one complete
group in the existing input window before modifying output.

A group has 1 control word, up to 30 tokens, M match descriptors, and
30-M literal words at maximum token count. Its length is
1 + ceil(M/2) + (N-M), with N <= 30, hence at most 31 words.

At group boundaries AC11=0. Read, validate and count all matching flag
bits from the control word, calculate total group length, and reject invalid
control fields before decoding any token. If the group is truncated by the
current input window, return +1 without consuming its prefix or changing
output. AC3 remains at the group control word and AC4 counts retained words.
The VFS frontend compacts these at most 30 pending words to the beginning
of its existing 64-word window, appends new data, and resumes the decoder.
After processing a group, advance AC3 and subtract its full length from AC4;
leave AC11=0. A complete group never requires a mid-token refill, so no
extra persistent descriptor cursor, control bitmap, or odd-halfword state is
needed across calls. VFS must treat EOF with a pending partial group as an
error and verify exact stream consumption at success.

Use AC0..AC7 as temporary per-group cursors and counters.
Keep AC10/AC11 externally valid at every return. The C-ABI VFS entry still
saves/restores AC10..AC15; it must also preserve any additional callee-saved
register it borrows under the actual compiler ABI, without increasing the
64-word refill window or standing memory. The v058 frontend uses only the original 72-word stack frame.

## Correctness gates

- Validate exact control count, matching flags, descriptor-pair padding,
  token kind, nonzero lengths, distances and output bounds.
- Support both even and odd 18-bit halfword distances, including overlapping
  distance-one copy. Reject before-origin references and input/output alias.
- Prevent 18-bit pointer wrap, count overflow and speculative output writes.
- Treat successful decoding as valid only with exact output and source counts;
  any malformed/truncated input must not be executed or published.
- Test every possible 1..30-word group split across a 64-word VFS window,
  repeated partial refills, final short read and fully in-memory Stage-1 input.
- Compare pre/post decoder code words, VFS stack high-water, Stage-1 copied
  image geometry, fixed-address low-core footprint, and final KCORE endpoint.
  Reject any net increase of persistent RAM or the 64-word refill window.

The single staged v058 fixed-address core and VFS frontend passed native
PDP-6 odd-halfword, every-prefix 31-word group, repeated two-word refill,
malformed control/padding, and copied-000060 image tests in daimos-testkit.
They are still NOT production-linked. Remaining validation includes address
wrap, source/output aliasing, exact stack/high-water measurements and all
filesystem/boot consumers. Only after full validation should an atomic switch of
KINIT, boot loaders, DXR/EXEC, PURE, TSFS and all host producers proceed.
