# K02 permanently refused session HUP and fatal-EXEC audit v1728

Date: 2026-11-20. Status: PDP-6 source-linked session refusal PASS.
K02 and K04 remain OPEN.

## Production source audit

The unchanged proc_session_teardown scans session members and skips
only the exact committed TRANSITION + VM_NONE + FINALIZE state.
Other members must accept fatal HUP through proc_event_apply. If a
member's FILE close refuses, event_apply returns an error and leaves
its FD and VM/u-area owned. The session leader must preserve the
attached TTY and retry later rather than claiming completed teardown.

The unchanged proc_event_apply refuses foreign fatal delivery for a
u-area with PROC_FATAL_CLOSE_BIT or an excluded transition. This
protects a process sleeping on its private fatal-close stack from
foreign HUP/TERM reclaiming its FILEs or the stack it will resume.
For a non-protected foreign user return it invokes
file_close_all_at_v865, and an error prevents proc_finish_slot.

For destructive low-memory EXEC, an irreversibly destroyed old VM
requires a fatal owner-side continuation, never a resumed user PC.
Independent v1720 and v970 regressions cover that handoff and owner
protection; physical v1727 separately covers permanent provider
refusal and private-stack bounded backoff. These observations do
not yet constitute a combined full-system fatal EXEC/session test.

## New source-linked PDP-6 regression

Versioned daimos-testkit files:

- tests/validation/daimos-k02-permanent-session-v1728.sh
- tests/validation/daimos-k02-permanent-session-v1728.s

The runner extracts the production session-teardown and event-apply
assembler functions into a native PDP-6 test image. A deterministic
test-only FILE-close stub refuses every attempt, retaining a stable
FD6 and the unchanged target u-area. The fixture contains a leader
in slot1, slot2 already committed to VM_NONE + FINALIZE, and slot3
with an ordinary live FILE FD6 under the same session.

Five session-HUP calls each fail after attempting to close only
slot3's FD6. Slot2 must not be revisited, neither target may be
finalized, and the TTY record cannot be cleared. The fixture then
sets the existing PROC_FATAL_CLOSE_BIT on slot3, representing its
private fatal-close continuation. Three more session-HUP calls fail
without another FILE close attempt or any loss of FD6 ownership.
The TTY remains attached throughout all eight refusals.

Three source-mutated unsafe controls must fail acceptance:

- Ignore a refused member HUP and proceed toward TTY release.
- Invert the protected fatal-owner branch in proc_event_apply.
- Redeliver HUP to the committed VM_NONE + FINALIZE member.

All three unsafe variants were rejected by the same native witness,
while the production source-linked positive case passed.

## Coverage limits

The v1728 test links real session/event production assembler but
models permanent FILE provider refusal at the foreign FILE close
boundary. It is NOT a booted multi-process guest or an organic D6FS
media failure. Physical v1727 has separately proved permanent D6FS
provider-entry refusal, retained private-stack owner, backoff and
independent HUP progress. Existing v922 repeatedly refused session
HUP and committed-member skip is an independent native control.

Remaining K02 work: a combined full-system session-HUP plus permanent
provider and fatal-EXEC interleaving. Remaining K04 work: policy for
permanently unreleasable resources without losing the live owner or
its private stack, adding an unbounded retry loop, or introducing an
unbounded resource reclaim. These cannot be called solved by v1728.

Production KCORE text delta: 0 words. Permanent RAM delta: 0 words.
Per-process RAM delta: 0. No production source or ABI modified.
K02 and K04 remain OPEN, K03 automatic OOM killing disabled.
pdp10-doc stays unchanged while the work is incomplete.
