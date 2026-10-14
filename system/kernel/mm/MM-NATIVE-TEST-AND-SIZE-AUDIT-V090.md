# Native PDP-6 MM testing and code-size audit v090

All permanent algorithm tests live in `daimos-testkit/tests/native-kernel/`.
Execute the **actual production `mm_pdp6.s`** in the PDP-6 simulator with
minimal deterministic dependencies. Do not substitute a host-C allocator.
Each algorithm has an independently runnable test, including successful,
invalid, boundary, corruption/rollback, and cross-algorithm interaction cases.
Use a separate failure code for each assertion. A test may use stub hooks for
unrelated subsystems, but must never stub its own algorithm under test.

## Test and optimization register

| Algorithm / entry | Native test | Inspection targets | Priority |
| --- | --- | --- | --- |
| `mm_find_fit`, `mm_alloc_aligned_noreclaim`, `mm_alloc` | `alloc-v090` (started) | repeated alignment validation, low/high gap scans, unnecessary moves and call frames; preserve best-fit preference | high |
| `mm_free`, `mm_find_base`, `mm_delete` | `free-v091` | duplicate linear scans, shift loops versus BLT, common error tails; preserve sorted table | high |
| `mm_pin_adjust`, `mm_pin`, `mm_unpin`, `mm_is_pinned` | `pin-v092` | share leaf paths, avoid duplicate metadata reads, immediate masks; prove overflow/underflow | high |
| `mm_find_fit` fragmentation classification | `fragment-v093` | share tail and interior-gap arithmetic, align masks, minimize branches | high |
| `mm_move_extent`, `mm_compact` | `compact-v094` | BLT copies of metadata and saved ACs, relocation direction, repeated scans, PI critical-section length | highest correctness risk |
| `mm_alloc_aligned` reclaim loop | `reclaim-v095` | avoid repeated frame construction, simplify retry bookkeeping; verify bounded callbacks | high |
| arena iteration / boot-populated `mm_arenas` | `arena-v096` | common tail paths, unnecessary per-arena recalculation; boundary checks | medium |
| `mm_extent_insert` & 32-descriptor limit | `descriptor-v097` | shift with BLT where safe, capacity tests, failure atomicity | high |
| MM + packed transaction | `packed-v098` | avoid extra buffer and duplicate lookups, confirm source pin and publication atomicity | high |
| deterministic operations with oracle | `stress-v099` | quantify frequency, branch behavior, descriptor fragmentation | high |

`alloc-v090` already executes the production assembly and passes basic low/
high allocation, pin-protected free, owner checks and gap reuse; separate
free/pin boundary suites are still necessary. Versions are test identifiers,
not a claim that all tests above exist yet.

## Equal-cost canonical PDP-6 assembly conventions

These are *preferences*, not semantics-preserving rewrite permissions. Apply
only after source/data dependencies, AC clobbers and instruction count match.
They are intended to give `p10fold` repeated identical byte-for-byte word
sequences that can be merged without growth.

1. **Entry/exit:** use the same AC save ordering (ascending AC10..AC16),
   frame-size expression, and matching reverse restoration across functions
   with identical live-register sets. Prefer one shared return tail per
   function where branching to it is no more expensive than duplicated code.
2. **BLT register blocks:** prefer a BLT for contiguous saved AC ranges when
   its setup plus the BLT is smaller and fast enough. Its source/destination
   and overlap behavior must be proved; a BLT of two descriptor words is not
   automatically cheaper than two MOVEs. Never let a copy corrupt the live
   source range or cross the pushdown-stack frame.
3. **Small constants:** canonical octal spelling, canonical instruction form
   and operand placement when mathematically and architecturally identical.
   Do *not* interchange `MOVE` with `MOVEI`, `CAIE` with `CAME`, or memory
   `SKIP` with immediate tests: the PDP-6 semantics differ.
4. **Conditionals:** put the common hot path on fall-through only if measured
   no worse; normalize equality/error branch direction where equal cost.
   Preserve skip-over-next-instruction contracts, especially around calls.
5. **Address/metadata operations:** use a consistent unpack order (span base,
   span count, metadata type, owner, pins) where dependencies allow. Prefer
   the same shift/mask sequence and scratch AC assignment in equivalent
   paths. Do not reorder memory accesses across observable publication.
6. **Calls/tails:** use identical calling sequences for identical signatures;
   share error returns and tail calls only when AC callee-save, stack and
   interrupt-state invariants hold. Do not introduce jumps on hot paths merely
   to create fold candidates.
7. **Critical sections:** retain ordering around `mach_pi_disable`, memory
   relocation, extent publication and `mach_pi_restore`. Never fold across
   transitions which require distinct interrupt or memory-owner states.
8. **Data placement:** canonicalize duplicate literals or constants only
   when addressability, relocation and cold/hot access cost remain valid.

## Size ideas to evaluate, not assumed savings

- BLT for runs of saved/restored ACs and for descriptor-table shifts, with
  source/destination overlap and direction investigated case by case.
- Merge identical validation and error tails; test if added branches cost
  more than recovered words or introduce a critical-path penalty.
- Remove duplicate `MOVE`/`HRRZ`/`HLRZ`/mask operations when AC values are
  already available, particularly in `mm_find_fit` and `mm_move_extent`.
- Choose common scratch-register layout and instruction order among similar
  routines to increase opportunities for exact `p10fold` suffix/prefix reuse.
- Replace repeated frame construction with cheaper canonical prologues where
  saved-set geometry permits it; keep stack high-water low.
- Remove unnecessary stack temporaries and reduce live ACs when proven.
- Assess high/low search sharing without introducing a branch per gap.
- Measure if a compact helper for small predicates (power-of-two alignment,
  owner/type check) saves *net* KCORE words after call overhead.
- Examine `JRST`/`JUMPx` chains for dead unconditional jumps and profitable
  fall-through. Weight execution frequency before adding shared tails.
- Investigate constant pooling, single-word literal forms, and relocation
  effects; compare both raw word count and post-`p10fold` count.

## Required result record per algorithm

Capture: git base SHA, simulator settings, test cases and independent oracle,
correctness results, raw KCORE words before/after, folded KCORE words before/
after, worst-case stack words, number of extra branches/calls on hot path,
and whether the revision increases latency or RAM usage. Reject a change
that adds words or worsens a hot path without a compelling correctness
reason. Prefer reducing **RAM consumption** over instruction savings;
execution speed is a close second. Disk output size is secondary.

Never optimize from a passing smoke test alone. Establish edge/stress
coverage first; then change one pattern at a time, run the native algorithm
suite and related integration regressions, measure, commit/push. If an
experiment fails, verify implementation details before discarding the idea.
No MM algorithm has been changed as part of v090's initial audit.
