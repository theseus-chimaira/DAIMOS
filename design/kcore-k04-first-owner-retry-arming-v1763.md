# K04: first-refusal registration with multiple pending owners v1763

Status: source correctness fix, isolated PDP-6 DAS assembly PASS.
K04 remains OPEN for physical multi-owner acceptance.

## Failure in the old shared-clock registration

`proc_exit_current` publishes FINALIZE|PREVM when abandoning the
private owner stack, **before** the first TTY/VM release attempt.
If a different slot has already failed, the shared retry word has
its pending bit set. The old `proc_finish_mark_pending_v892` then
treated the prepublished FINALIZE mark plus that global pending bit
as proof that the new EXIT owner had failed previously. Its first
failure consequently inherited the older owner's possibly capped
01000-octal-tick delay and stale retry epoch, rather than starting
its own 020-octal-tick backoff. This also made the source comment
that every new owner resets the shared interval inaccurate.

## Correction, with no new resident words

Word 067 is already shared by mutually exclusive process lifetimes:
ordinary swapped RUN owners use bit 32 for the swap-retry deadline,
while finalizing EXIT/DISCARD owners use bit 35 for FINALIZE.
In FINALIZE mode v1763 reuses bit 32 as the **per-owner armed mark**:

- FINALIZE without ARM means no failure has yet registered, even if
  other owners are already pending. Reset the shared clock epoch to
  the current timer and the retry delay to 020 octal ticks.
- FINALIZE|ARM plus shared pending means this owner is already
  registered. Preserve existing shared backoff and current epoch.
- When the global pending bit is absent, registration still resets
  the epoch, even for a marked owner. The finalizer stores ARM with
  the complete owner marker on both paths.
- A post-VM failure after VM destruction uses the same mechanism:
  the PREVM marker is cleared on success, so the first refused MM
  u-area release registers the new post-VM stage independently.

Rollback SIDL's no-u-area pre-VM marker and retry epoch remain
unchanged. The swap-in paths cannot consume this ARM bit on an
EXIT/DISCARD owner because those processes are TRANSITION-locked
and not eligible swapped RUN jobs. The exported FINALIZE/PREVM
diagnostic bits and low-half exit status remain unchanged.

Targeted source assembly and whitespace checks pass. Physical
acceptance remains outstanding: keep owner A permanently refused
until the shared delay saturates, create B with prepublished PREVM,
verify B's first refusal resets the epoch and delay, then verify
subsequent refusals preserve capped backoff; also check clock wrap,
WAIT/PID reuse, and the independent SIDL rollback path.
