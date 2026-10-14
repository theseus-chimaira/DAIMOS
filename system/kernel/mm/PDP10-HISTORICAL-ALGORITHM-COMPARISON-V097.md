# v097: Historical operating-system algorithm comparison — mandatory per-routine review

**Scope:** user-supplied TENEX (`tenex.tar.gz`), ITS (`its.tar.gz` and
`ITS-138.tar.gz`) and JOSS (`joss.tar.gz`). FUZIX is intentionally excluded:
it is not a PDP-10 operating system. These are historical *candidate ideas*,
not measured improvements over DAIMOS. The current v096 MM implementation is
the optimization baseline; no historical code has been imported.

## Rule for every future kernel algorithm

Before accepting a rewrite, look for the corresponding algorithm or resource
management strategy in **TENEX, ITS and JOSS**. Record the exact source file,
algorithm, memory representation, instruction-count feasibility and whether
its behavior can actually replace the DAIMOS contract. Where source is only
scanned documentation or a different architecture, state that limitation.
Compare permanent RAM, temporary stack, execution cost, post-fold code size
and correctness on the PDP-6. Never assume historical code is smaller.
Extend the per-algorithm native test fixture when an alternative changes
observable behavior or boundary conditions. Preserve this step when the
review proceeds beyond MM to the rest of the kernel.

## TENEX

Source inspected: `tenex-20260812-v1/134-tenex/FREE.MAC` (also supplied in
128, 133 and 135 revision trees). In `ASGFRE` it keeps a **linked list of
free blocks**, a free-space counter in the allocator header, a lock, and
scratch state. It first rejects requests larger than total free storage,
then traverses the free-block list looking for an **exact-size match** or a
best-fitting larger free block. It splits a larger selected block and updates
its predecessor's pointer. It also clears newly assigned storage using BLT.
`RELFR*` provides return/coalescing mechanics. `ASGPAG` is page-assignment
logic, not a directly equivalent physical variable-size-extent allocator.

**Relevance:** consider early total-free rejection, exact-fit fast path,
and BLT for sufficiently long moves. **Not automatically better:** headers,
locks, free-list links, fragmentation metadata and coalescing consume RAM
and instructions that DAIMOS's 32-entry, *allocated-extent-only* table does
not. TENEX best-fit would also violate DAIMOS's LOW-first/HIGH-highest
placement rule unless the rule and all callers were deliberately redesigned.
The TENEX routine does not provide proof that either DAIMOS routine can be
replaced with fewer than its current 85/71 words.

## ITS

Source inspected: `its/src/mudsys/core.13` and
`its/src/mudsys/nfree.54` from `its.tar.gz`.
`core.13`'s `.CORE` code manages **page/block-granularity process address
space** through ITS `CORBLK` calls, converting blocks to words via shifts;
it delegates physical memory allocation to ITS and is **not** a substitute
for DAIMOS's contiguous physical gap scanner. `nfree.54` is MUDDLE's
application-runtime storage manager (`CAFREE`/`CAFRET`) with free-list and
GC-oriented behavior, not the ITS resident supervisor's physical allocator.
Its `FREEZE` code notably uses a BLT to copy variable-sized objects.
Treat those techniques as useful comparisons for copying and allocation
classes, but do not count MUDDLE runtime code as kernel MM savings.

The separately uploaded `ITS-138.tar.gz` includes an archival ITS listing
as page images/PDFs. It has not been transcribed and independently
verified at the exact kernel allocator entry point in this pass; **no
instruction count or physical allocator claim is made from those images**.
A targeted listing search remains an open follow-up if needed.

## JOSS

The uploaded `joss.tar.gz` is primarily historical PDF documentation,
including `RM-5437-PR_JOSS_Assembly_Listing_of_the_Supervisor.pdf` and
`RM-5270-PR_JOSS_Central_Processing_Routines_Aug67.pdf`. The assembly
listing is not a directly parseable text source (`pdftotext` yielded no
usable text). Thus **no validated comparable JOSS gap-search or no-reclaim
allocator** has been identified; claiming JOSS is smaller would be
unfounded. JOSS is also a historically different supervisor and should be
compared by resource-management semantics, not assumed API equivalence.
Manual inspection of selected listing pages is needed for a fair comparison.

## Current conclusions by DAIMOS MM algorithm

| DAIMOS function | Historical lesson | Decision |
| --- | --- | --- |
| `mm_find_fit` (85 words v096) | TENEX list-based best/exact-fit versus DAIMOS sorted occupied gaps; ITS process pages are not physical gaps | Retain current representation; experiment with returning insertion slot in existing scan |
| `mm_alloc_aligned_noreclaim` (71 words v096) | TENEX prechecks aggregate free; lock/split mechanism heavier, different failure semantics | Keep non-reclaim ABI; measure fused gap+insertion scan |
| `mm_free` / descriptor shifts | TENEX linked free-list reclamation and splitting; DAIMOS implicit free gaps | Retain implicit gaps until a better metadata/size measurement exists |
| `mm_compact` / extent move | ITS block-level core resize is not relocation of resident contiguous process image | Require native data-preservation/PI tests; BLT is candidate for long descriptor/data moves only |
| packed storage / reclaim | TENEX page allocation and MUDDLE GC are materially different | Investigate pressure behavior, avoid importing GC/page policies as direct replacements |

**Status:** source-level comparison complete for TENEX `FREE.MAC` and the
specified ITS components; **JOSS and the ITS-138 archival listing are
incomplete**, explicitly tracked. No instruction-count comparisons were
made across incompatible assembler dialects or memory-management contracts.
