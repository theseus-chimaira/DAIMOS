# K04 v1801: process teardown recovery closure

Date: 2026-11-22. Status: **CLOSED** against the defined K04
ownership/recovery invariants. This is a bounded engineering
acceptance, not a claim of exhaustive hardware-fault coverage.

## Accepted behavior

The kernel keeps ownership of VM, u-area, FILE and session TTY
resources while a real provider refuses teardown. The private
owner stack finishes FILE close and HUP before the allocation-
free deferred MM finalizer can retire the process. PREVM EXIT,
post-VM FINALIZE, SIDL DISCARD and unpublished VM_NONE DISCARD
are distinguished, and refusal cannot make an owner prematurely
reusable. The shared reaper applies bounded exponential backoff
without blocking independent work or unrelated scheduling.

The existing physical acceptance reports establish:

- v1776-v1785: real temporary MM pins, two-owner mixed orderings,
  PREVM/post-VM and SIDL cleanup, exact owner release/reuse;
- v1787: qualified 17-bit clock wrap with one SIDL owner;
- v1793-v1794: independently refused SIDL/PREVM owners;
- v1795: permanently pinned VM owner, capped retry, responsive
  bystander and controlled operator HALT with CLEAN D6FS;
- v1796: real FILE descriptor with refused VFS SYNC on the
  owner's private fatal continuation, followed by recovery;
- v1797: dying parent, physically pinned child, healthy INIT;
- v1798-v1799: session HUP versus refused FILE owner and
  unpublished VM_NONE DISCARD owner, exactly one TTY release;
- v1800-A: child exits after parent's physically pinned PREVM,
  adopted by healthy INIT and WAITed while parent still owns VM;
- v1800-B: INIT itself has a real fatal-FILE refusal while
  parent has a physical PREVM pin; late child becomes FREE once,
  never an uncollectable ZOMB, before either owner releases.

No additional permanent KCORE code, RAM, process descriptor
words or ABI fields were introduced by v1800-A/B or closure.

## Final integrated regression

`daimos-testkit/tests/validation/daimos-k04-closure-regression-v1801.sh`
ran ten versioned independent verifier self-tests, including
their negative controls. It then reverified **both** accepted
v1800-A and v1800-B real PDP-6 post-HALT source-linked memory
readouts and their read-only `d6fsck -n 4` results. All PASS.
Both D6FS images are CLEAN (sequence 3); the guest shutdown
in each case was via privileged HALT, not a forced host stop.
The closeout regression is read-only and does not create or
alter a boot image. Earlier accepted full-system physical runs
remain the underlying evidence; the closeout does not pretend
to have rerun every physical fixture.

## Deliberately bounded residual risks

The original v1771 acceptance plan listed combinations broader
than needed to establish the ownership invariant. The following
were **not** independently reproduced and are not represented
as passed:

1. Mixed two-owner 17-bit wrap (single-owner physical wrap and
   multiple-owner independent backoff were tested separately).
2. Session HUP with a physically pinned *published* PREVM or
   post-VM member (FILE refusal and physically pinned VM_NONE
   DISCARD session members were tested separately).
3. Organic D6FS/controller/media failure (FILE failure was
   injected at the real VFS SYNC provider boundary).

These are residual fault-combination and environmental coverage
risks, not currently demonstrated KCORE defects. They remain
eligible for targeted regression if a related bug or provider
change appears; they do not block bounded K04 closure. The
permanent-refusal policy intentionally **retains** the exact
owner until recovery/operator intervention. It never silently
reclaims an unreleased MM extent. K04 closure therefore does
not authorize arbitrary OOM victim killing: K03 must separately
prove safe allocation-free victim selection and reclaim policy.

## Change control

K04 is closed for the current production implementation. Any
future modification to the finalizer, FILE/HUP private stack,
VM ownership, retry epoch, adoption or WAIT rules must rerun
the relevant targeted fixtures. The integrated v1801 verifier
suite is the minimum read-only closeout check. No production
KCORE changes are required by this closure.
