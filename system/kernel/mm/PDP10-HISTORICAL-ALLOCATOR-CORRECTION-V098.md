# v098 — Corrected primary historical comparison: ITS-138 and JOSS

## Source priority and correction

Use the **uploaded `ITS-138.tar.gz` as the primary ITS reference** for every
future kernel routine review. Its `ITS-138/transcribed/ITS_138/pageNNNN.txt`
files contain actual assembly-listing transcriptions. The separate `its.tar.gz`
contains a later ITS version, not a substitute for ITS-138 and **must only
be used as an explicitly identified later comparison**. The v097 comparison
incorrectly treated an ITS-138 listing as unavailable as text; its reliance
on later ITS/MUDDLE routines as the principal ITS reference is superseded.

TENEX is paged; its `FREE.MAC` allocator is not a directly interchangeable
physical contiguous-core allocator. It can still inspire data-structure or
instruction ideas, but comparisons must charge page-map/pager dependencies,
extra free-list metadata, and changed allocation semantics. Do not claim
that TENEX best-fit is cheaper than the nonpaged DAIMOS design.

## ITS-138: core allocator is explicitly present

Primary file: `ITS-138/transcribed/ITS_138/page0340.txt`, heading
`SUBTTL CORE ALLOCATOR` (page 34). The routine begins at `UACORE`/`ACORE`.
Continuation: `page0350.txt` through at least `page0380.txt`.

- `ACORE` handles user core requests, distinguishes existing allocation,
  growth and reduction, and coordinates with a per-user `MEMTOP` and `UPR`.
- `MEMFR` is consulted to determine whether adequate free core exists;
  `MXCCA` finds a maximum contiguous available core region (`page0350`,
  source lines 3–20). `ACMS2`/`ACMS7` scan block owner/status metadata
  `MEMBLT` and classify users, executive, I/O, and free blocks
  (`page0370`, lines 19–30; `page0380`, lines 2–9).
- `MEMFRZ`, `CRACF`, and `BEXC`-related state coordinate allocation and
  inhibit relocation of selected users. `ACMS2` participates in a
  relocation/shuffle when contiguous space is unavailable. Core updates
  use `CCFRE`, `MLO`/`MLU` and `CZRR` (`page0370`, lines 2–16).
- Its accounting and relocation operate at `MX1KB` block granularity,
  rather than DAIMOS's arbitrary-word-size sorted allocated extents.

**Potential lessons for DAIMOS:** preflight total-free/core-contiguity
checks; block-owner maps as an alternative to extent lookup; explicit
relocation exclusion; contiguous-region discovery and selective movement.
**Costs and limitations:** per-block ownership/status tables, additional
rearrangement machinery, interrupt coordination and 1K-block rounding.
The historical core allocator spans multiple listing pages, so it is not
credible to compare its *whole* word count to DAIMOS's 85-word gap helper.
`MXCCA` and the scan may deserve a separately bounded source-size analysis.
No assembly code has been ported or tested, and no superiority is claimed.

## JOSS: verified dynamic core management design

Primary source: uploaded `joss.tar.gz`, document
`RM-5216-PR_JOSS_User_Scheduling_and_Resource_Allocation_Jan67.pdf`,
section **III.6 Dynamic Memory Assignment** (printed pages 41–44), and
its discussion of **core compaction** in the main processing loop.
The accessible text in this report explicitly describes:

- `CORE`: one entry for each **1,024-word user-core block**, storing a user
  busy bit, a drum busy bit, and a user number.
- `S.S`: a user status table containing each user's core location and
  required block count. The redundant reverse association avoids searches.
- Multiblock user storage must be **contiguous** to support its internal
  linked structure and avoid excessive interpreter overhead.
- To swap in a large user, smaller users may be sent to drum by priority;
  if the resulting free blocks are noncontiguous, the main processing loop
  performs core compaction while the interpreter is not using those blocks.
  Physical relocation and status publication are coordinated.

**Potential lessons for DAIMOS:** cheap O(1) owner lookup via a reverse
physical-block map; block-by-block movable/pinned classification; deferring
compaction to a safe scheduler point; storing residency and transfer state
compactly. **Tradeoffs:** block map consumes proportional RAM and constrains
allocation granularity; 1,024-word blocks waste space for small, exactly
sized packed process images. A fair comparison must count these costs.

The supplied `RM-5437-PR_JOSS_Assembly_Listing_of_the_Supervisor.pdf`
contains the supervisor listing, and `ocr/super-ocr.pdf` is searchable, but
an exact JOSS gap-search assembly entry point, instructions and standalone
word count have **not yet been verified**. Do not invent them. Consult the
listing before claiming a source-level JOSS optimization.

## Comparison and next evaluation

| Property | DAIMOS v096 | ITS-138 | JOSS |
| --- | --- | --- | --- |
| Physical allocation unit | Arbitrary words | Block-granular (`MX1KB`) | 1,024-word blocks |
| Allocation metadata | 32 sorted occupied extents, 2 words each | `MEMBLT` block state and user tables | `CORE` block map and `S.S` user status |
| Gap selection | LOW first or HIGH highest aligned fit | `MXCCA` contiguous-core search; relocation | contiguous free core blocks; compact when needed |
| Compaction exclusion | pinned process extents | `BEXC` / allocator coordination | main-loop interpreter-safe scheduling |
| Proven smaller than DAIMOS? | Baseline | No | No |

**Decision:** preserve the DAIMOS allocation policy for now. Measure whether
ITS-138's `MXCCA` can be distilled into a smaller bounded gap scanner and
whether JOSS-style reverse block ownership is worth RAM cost under
32K/256K configurations. Test alternatives against the real PDP-6 allocator
in daimos-testkit and compare whole-system memory, not merely instructions.

For all future routine audits, prioritize **ITS-138 and JOSS**; TENEX is
secondary and must be qualified as paged. Mark later ITS explicitly and do
not use it to stand in for ITS-138.
