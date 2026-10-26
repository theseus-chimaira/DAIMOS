# Proposal B v409: proposed per-process vnode lock transitions

Status: review proposal only; NO production semantics or ABI approved.

## Validated bit placement

The candidate from v408 uses bit 35 READ, 34 WRITE, 33 DIR/APPEND,
22..21 regular/lock state and bit 23 reserved; canonical vnode mask
077707777777 and metadata mask 700070000000 (octal). All six bit
positions were independently checked with integer shifts. The candidate
requires provider and local kind to fit 0..7. The full 36-bit offset
continues in descriptor word one.

## Proposed invariant

Within any ONE process, all open regular-file descriptors for the SAME
full canonical vnode have an identical lock mode (unlocked, shared or
exclusive). There is no stored lock-family token and no promise of
interprocess exclusion. READ/WRITE/APPEND remain descriptor-local.

## Operation transition proposal

OPEN regular vnode: validate and create a descriptor; if another regular
descriptor for that canonical vnode already exists, copy its lock state;
otherwise initialize unlocked. Permission and APPEND are always those
requested by this OPEN, not inherited from the other descriptor.
Publication must occur after all failure-prone path checks. Different
mount IDs never alias even when provider, kind and local index agree.

DUP and DUP2: copy both words, including lock state and independently
copied offset. DUP2 first CLOSEs destination, preserving its old record
on CLOSE failure; an existing same-vnode family has matching state by
the invariant. Pipe reference increments and failure handling remain.

FLOCK on regular FD: SHARED/EXCLUSIVE/UNLOCK set that mode on every
matching full canonical vnode in the current process. The old
independent-OPEN family conflict is deliberately removed. No other
process is consulted. Nonregular FDs are rejected as today.

CLOSE successful: retire just that descriptor; surviving same-vnode
records retain the lock. CLOSE failure: preserve both metadata and
vnode/offset and do not affect peers. Last close removes all local lock
state for that vnode. Unmount clearing touches only matching mount-ID
records, preserving other mounted vnodes even with identical local IDs.

RUN: select parent descriptors using the explicit fd map, copy their
complete first/second words to child slots, and publish child only on
successful mapping/pipe-ref completion. Multiple mapped source FDs for
one vnode carry the same lock state if parent invariant holds. Mapping
only one member is allowed: child receives its mode independently;
subsequent changes in parent or child do not propagate. A failed RUN
must discard staged child state and undo any acquired pipe refs.

EXIT/forced teardown: close all descriptors using existing cleanup
contract; no separate lock-object lifetime remains. FIFO substitution
must update vnode identity without inheriting unrelated regular locks.

## Critical limitations

This intentionally breaks the current test asserting conflicting locks
between separate OPENs within one process. It is a proposed change, not
compatible compression. The absence of global ownership makes this
lock more of a local advisory state than an interprocess synchronization
primitive. Whether this satisfies DAIMOS's necessary locking capability
requires an explicit decision. The generic VFS still accepts provider/
kind 0..63 while the proposed FILE layout only accepts 0..7; enforce or
redesign this boundary before producing code.

## Acceptance sequence

1. Approve or reject local vnode-wide FLOCK and narrow provider/kind.
2. Define tests in daimos-testkit for OPEN inheritance, independent
   permissions/append/offsets, DUP2 failure, RUN map/failure, FIFO,
   26/63 mounts, unmount/exit, and explicit absence of cross-process
   lock guarantee; revise incompatible existing FLOCK tests deliberately.
3. Prepare a single coherent prototype with a linked-image baseline;
   measure permanent, process/peak MM words and hot READ/WRITE paths.
4. Accept only a demonstrated total RAM benefit with correctness.

v388 reference only: KCORE 9109; KCORE+MRES 15657;
PERMANENT_LAST 15704. No new implementation, size result or test pass.
