# v1672: native zombie accounting and physical PURE RUN/EXIT/WAIT pressure

Date: 2026-11-18. Accepted DAIMOS parent v1671:
`19a348898044c441e6cd5d2ccbd09d7e8d361d0d`.

## Priority fix: a zombie is not a 0777777-word user image

Post-user physical memory-pressure testing uncovered a real error
in permanent `system/kernel/proc/syscall_info.s`. Normal EXIT
destroys a process VM, recording `VM_SPACE_NONE` as the all-ones
36-bit word (-1); a zombie may retain its process-table slot
until WAIT. Both `sys_meminfo` and `sys_procinfo` extracted the
left 18-bit value `0777777` from that sentinel and treated it as
allocated/logical user words. A real 48K four-child fixture,
before the fix, reported `PROCESS-WORDS 532246` when two children
were zombies awaiting WAIT, despite only 49152 installed words.

The actual native assembly now guards *only* the 0777777 high
half: `SYS_MEMINFO` skips it when summing user image words, while
`PROCINFO` reports zero logical words for the zombie. Valid
resident and swapped, non-sentinel logical images retain their
original counts. The change is four fixed KCORE instructions,
no new data word and no per-process allocation. The original
PDP-10 `CAIN` conditional instruction is used correctly: it
skips its next instruction if the value is NOT equal. `CAIE`
would reverse this policy and fail the native correctness test.

The daimos-testkit source-extracts both real assembler leaves,
executes them natively with p10run PDP-6 and independently
rejects two mutations that remove the sentinel guards. Both
positive and both negative suites PASS. The corrected KCC FULL
ROOT=auto KINIT also links with both MRES candidates present.

| Fixed KCORE end | Previous v1671 | Corrected v1672 | Growth |
| --- | ---: | ---: | ---: |
| GCC lowmem, octal | 024076 | 024102 | **4 words** |
| KCC FULL, octal | 024353 | 024357 | **4 words** |

The new low resident occupancy in the physical lowmem test is
**17729 words at 48K**, **17745 at 64K**, four above the prior
configuration. This is a necessary correctness fix; the separate
TSFS swapless-only image still saves the same **1877 transient
GCC image words** against a general image from the same source
(`113736` -> `110211` octal).

## Physically tested real RUN/EXIT/WAIT on 48K and 64K

In the separate daimos-testkit only, a small, source-built C89
fixture replaces `/SYSTEM/INIT` on a disposable three-member
TSFS root. A tiny user assembly SLEEP-then-EXIT program replaces
`/SYSTEM/EXEC/MOUNTALL`, strictly in those disposable test media.
This avoids conflating the workload with LOGIN/DSH, while still
exercising the real kernel RUN, scheduler, process APR/u-area
allocation, EXIT and WAIT code. Ordinary userland INIT, root
images, kernel and PURE lifecycle sources remain unchanged apart
from the zombie accounting fix above.

The fixture launches **four concurrent real children at 48K**
or **seven at 64K**, retains their live PIDs, then WAITs each
clean native EXIT(0). It deliberately holds at three user-space
CTY input barriers: baseline A, all children present B, and
all children reaped C. The host pauses SIMH with CTRL-E at each
barrier, examines the original physical KCORE MM arena and extent
tables and releases the guest. SIMH execution breakpoints MUST
NOT be placed at user virtual addresses; those numerical PCs
collide with unrelated physical KCORE instructions and yielded
false previous snapshots. All four final test scenarios use the
explicit CTY protocol with the simulator boot script's trailing
`exit` suppressed, and private physical copies of all four
DECtapes. AUXSTORE DSK/DRM images are detached throughout.

Each original-MM snapshot validates installed core, physical
arena extent bounds and order, two-word extent ownership and
unique process APR owners, active pins, exact free+allocated
managed-area conservation, and permanent low-resident count.
The tests refuse wrong process counts, failed RUN/WAIT, and a
failure to restore the exact baseline owner/extent table after
WAIT. Four deliberately invalid physical/report variants are
rejected: corrupted MM arena, missing CTY barrier release,
fabricated free total and duplicate APR owner. No synthetic
process allocation is substituted for native RUN.

### Actual physical PDP-6 results

| RAM | Configuration | Baseline free | Child-pressure free | Post-WAIT free |
| --- | --- | ---: | ---: | ---: |
| 48K | General and strict | 27441 | 18193 | 27441 |
| 64K | General and strict | 43793 | 27609 | 43793 |

| RAM | Children | Physical APR owners | MM allocated words | Transient words consumed |
| --- | ---: | --- | --- | ---: |
| 48K | 4 | 1 -> 5 -> 1 | 3934 -> 13182 -> 3934 | **9248** |
| 64K | 7 | 1 -> 8 -> 1 | 3950 -> 20134 -> 3950 | **16184** |

Each child owns an exact **2048-word user APR** plus a **264-word
kernel dynamic allocation** (the stable u-area), for a total
**2312 words per live child** at these checkpoints. The original
PID1 retains its 3072-word APR. The kernel retains one baseline
256-word pinned extent; its physical owner, location and pin
count remain unchanged before/during/after pressure. The largest
contiguous MM free extent equals the free-word total at all three
checkpoints, for both builds and core sizes.

General KINIT carries the dormant disk-swap MRES candidate,
strict KINIT prunes it, but both install the SAME PURE MRES.
Consequently, real post-user process-pressure, all APR owners,
free words, largest free block and pinned ownership are exactly
identical at all three checkpoints. The boot transient saving
from v1668/v1670 does not turn into additional runtime free RAM.
The corrected physical SYS_MEMINFO reports 3336 process words
before/after the 48K workload and 12584 under four-child
pressure, matching the difference in actual physical MM owners.

## Measurement boundaries and remaining work

The test checks three stable physical points in true native
post-user execution, not every instruction or every scheduler
interleaving. It does NOT establish a continuous post-user
high-water, or prove forced reclaim under pressure or compressed
bitmap mode3 process creation, restore, PI6, or teardown. The
children were retained concurrently without disk swap and were
reaped normally, so it validates the RAW PURE runtime behavior.
Production PURE stays opt-in, bitmap mode3 stays production OFF.
Strict physical 32K and other root-class physical work remain
separate gates. Full evidence and positive/unsafe test code are
under daimos-testkit `tests/validation/pure-runtime-pressure-v1672/`.
