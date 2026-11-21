# K04: retry-clock phase ordering after first-owner arming v1765

Status: source correctness fix; isolated PDP-6 DAS assembly PASS.
K04 remains OPEN for physical fault-injection acceptance.

## Concrete failure path

v1763 made the first refusal of each FINALIZE owner reset the shared
retry epoch to the current clock and its interval to 020 octal ticks.
However, the slot-zero idle loop called the backoff-advance helper
**after** its entire reaper scan. If a pending PREVM owner completed
VM teardown during a scan but then encountered its first post-VM
u-area MM refusal, that new stage correctly armed its interval at
020 -- only to have the caller immediately double it to 040.

The same error applied when an unarmed, newly registered owner was
first encountered in the scan while another owner was already
pending. The backoff no longer reflected the new owner's initial
020-tick delay. This did not free memory incorrectly, but violated
the bounded retry schedule and delayed recoverable owners.

## Correction

Advance the existing shared delay **before** starting the slot-zero
scan, while PI6 is inhibited. Existing refused owners still follow
020, 040, 0100, ..., 01000-tick exponential delays. A new failure
encountered during the scan can reset the epoch and delay to 020,
without any subsequent doubling in the same pass. If all owners
complete, the service clears both the pending bit and now-unused
delay in the original one-word retry clock, preserving its low-half
epoch. Registration of a later owner always starts a fresh interval.

No change to resource-release ordering, per-owner FINALIZE/ARM tags,
reaper completion ABI, process table, or permanent RAM footprint.
The existing assembly source passed native DAS and diff checks.
No testkit files or physical guest have been run.

Physical acceptance remains necessary for a capped permanent owner
A with another owner B newly entering POSTVM during the scan,
first and repeated retry clock intervals, successful independent
release, timer wraparound, and WAIT/PID reuse.
