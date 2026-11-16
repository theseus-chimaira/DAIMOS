# P1-E v1633: physical 48K APR corridor lower bound

Date: 2026-11-16. Base: committed/pushed v1632 source-identical
five-child PURE-ON/PURE-OFF physical owner measurements. No
production allocator, process, MRES or KCORE implementation edit.

## What the original PDP-6 extents prove

The testkit v1633 physical auditor uses the complete original
v1629 enabled MM table at parent B and v1632 disabled MM table
at five-ready. It authenticates both source-identical KCORE maps
and INIT images, then checks the unpinned five PACKED owners,
the fixed parent APR and low-core necessary dynamic extents.
The testkit rejects a fabricated low-core dynamic extent and
a fabricated parent extent.

| Original MM property, octal words | PURE enabled | PURE disabled |
|---|---:|---:|
| Necessary low-core dynamic high | 046605 | 043247 |
| Next legal APR-aligned start | 050000 | 044000 |
| Fixed parent APR | 126000..136000 | 126000..136000 |
| 010000-word APR cells before parent | 5 | 6 |
| Free tail above parent | 002000 | 002000 |

An additional full 010000-word process cannot fit above this
parent. Six complete runnable child images below this parent
would require the first APR cell at 046000, whose end at 056000
is followed by five further 010000 APR cells ending at 126000.
Thus necessary low-core occupancy must end at or below 046000;
the actual PURE-enabled boundary 046605 exceeds it by precisely
**000605 octal / 389 decimal 36-bit words**. This is a physical
geometry lower bound for the *fixed* parent and dynamic objects.

The installed optional PURE service takes 003336 octal / 1758
decimal words. Its absence in the otherwise identical disabled
guest explains the exact 003336 shift of the low-core dynamic
boundary. An implementation that reduces the necessary low-core
high-water mark by at least 389 words, with unchanged physical
process/other allocations, could cross one APR alignment boundary.
That is a hypothesis to be checked in a new linked and booted
image, not an implemented solution or a guarantee of six RUNs.

The existing `mm_compact` deliberately moves only stopped
MM_TYPE_PROCESS extents. PACKED suffixes have no resident APR
base and cannot be moved without atomically updating their
authoritative owner/restore geometry. Even relocating such
suffixes cannot make six *simultaneously runnable* full APRs
fit between the unchanged low-core boundary and fixed parent.
It would be unsafe to claim a fix from more aggregate free RAM,
from repeated allocation attempts, or from moving MM metadata
without its words. Do not weaken source/PI6/failure-atomic rules.

## Next implementation decision

First investigate whether a correct reduction of 389 or more
resident low-core words is possible in the removable PURE MRES
without deleting full restoration, breaking fail-closed rollback,
adding permanent state or growing fixed KCORE. Prefer carefully
verified shared code or already-proven fold targets; test original
source/C compiler and linked code before claiming byte savings.

Separately, a full-source physical v1391 relocated restore and
transient peak/net RAM proof remain open. The v1633 bound is
not a blanket theorem for a dynamically relocated parent or a
future different MM representation. PURE default OFF and strict
32K optimization HOLD; production code unchanged in v1633.
