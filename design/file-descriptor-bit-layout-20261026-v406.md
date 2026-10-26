# Proposal B v406: exact packed FILE bit budget

Status: design analysis only; no production ABI or filesystem change.

## Canonical representation versus actual configured range

`vfs.h` defines a full 36-bit vnode with provider:6, mount:6,
local-kind:6, index:18. The current FILE code explicitly limits both
provider and local kind to 0..7 and mount ID to 0..31 (with planned
public mount capacity through 26). Under those limits:

  bits  0..17: 18-bit vnode index (all used)
  bits 18..20: 3-bit local kind
  bits 21..23: metadata available
  bits 24..28: 5-bit mount id (0..31)
  bit      29: metadata available until mount 32..63 is supported
  bits 30..32: 3-bit provider
  bits 33..35: metadata available

For the current 26-mount target the raw canonical vnode mask is
`073707777777` octal and the available metadata mask is
`704070000000` octal (both as 36-bit values). The complement calculation is the authority; verify literal
width and target assembler acceptance before using any mask in code.
For 63 mounts, canonical mask becomes `077707777777` and only six
metadata bits remain. These are restricted-profile storage layouts,
NOT general representations of arbitrary six-bit providers/kinds.

The current metadata fields are: READ at bit 35, WRITE 34, DIR/APPEND
33, family at bits 29/28/27/23, and regular/lock state at 22/21.
Mount bits 27 and 28 therefore collide with the existing family token.
Simply changing FILE_NODE_VNODE_MASK loses family identity, while simply
dropping the u-area extension loses mount identity.

## Capacity and consequences

For 26 mounts, available metadata = 3 + 1 + 3 = 7 bits per descriptor.
Existing direct fields require 1+1+1+4+2 = 9 bits. Thus at least two
bits must be removed, encoded differently, or stored separately.
For future 63 mounts available metadata = 3+3 = 6 bits; at least three
bits must be removed, encoded differently, or stored separately.

Seven metadata bits permit, for example, READ, WRITE, DIR/APPEND,
two lock-state bits, and two family bits, but TWO FAMILY BITS CANNOT
uniquely identify 16 concurrent OPEN families. It is an invalid
compatible layout, not an implementation recommendation.

The exact masks above are an encoding feasibility result only. No
code-size, instruction-latency, per-process or MM allocation saving has
been demonstrated. Any accepted redesign must preserve 36-bit offsets,
independent DUP/RUN offset copies, provider/PIPE distinctions, and lock
ownership or explicitly replace the FLOCK contract with approval.

## Next gate

Audit actual provider/kind maxima, and develop a six-bit (not just
seven-bit) mode/locking contract so 63-mount expansion is possible.
Enumerate whether the family token and regular/lock state can be
simplified without losing required functionality. Then stage a coherent
production prototype and test only after approval, using full linked
PDP-6/PDP-10 measurements and daimos-testkit.

Reference baseline v388: KCORE=9109, KCORE+MRES=15657,
PERMANENT_LAST=15704. Strict 32K acceptance and JRST folding deferred.
