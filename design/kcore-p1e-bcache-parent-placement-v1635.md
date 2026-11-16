# P1-E v1635: original BCACHE high-slab obstructs first parent APR

Date: 2026-11-16. Base: v1634 byte-equivalent GCC removable-MRES
compiler experiments, with no viable 389-word compiler-only shrink.
This report is read-only physical PDP-6 diagnosis, not a code change.

## Original first-VM physical snapshot

The exact original v1629 48K PDP-6/TSFS/no-AUX/PURE root tapes
and compiled guest were replayed with a source-linked SIMH breakpoint
at the first `vm_space_create` entry, PC 022472 octal. Current
process slot is zero, high slot is two; this is process 1's first
VM allocation. The requested 007067 logical words round to a
010000-word aligned APR. Three MM arenas are present, with the
highest covering 114222..140000 octal words.

**Before the parent VM is allocated**, the MM owner table already
contains `137577..140000` as an unpinned 0201-word KERNEL_DYNAMIC
extent, owner 6. Original `system/kernel/storage/bcache_pdp6.s`
authenticates owner 6, the private clean cache's tag + 0200 data
words, and explicit allocation preference MM_ALLOC_HIGH. The
slab physically occupies the highest 0201 words of 48K core.

With that slab retained, a 010000-word APR aligned at 02000 can
start no higher than 126000. Without the high slab, the exact
same unmodified arena and alignment policy permits placement at
130000..140000. The separate completed original v1629 MM B
snapshot confirms parent 1 actually remains at 126000..136000.
The difference is an entire 02000-word alignment quantum; the
small slab itself uses only 0201 words, but its high position
blocks an otherwise complete APR cell.

The new source/physical testkit auditor PASSes. It rejects two
forged snapshots that modify either the cache owner identity or
its physical extent length. No resident kernel, cache, allocator,
PURE module, executable or original test fixture was edited.

## Implication for 48K process admission

The v1633 389-word low-core improvement was a rigorous bound
**with the parent fixed** at 126000..136000. It does not require
shrinking the module if the first parent can instead be placed
at the available top APR. A candidate is to allocate the tiny
reclaimable clean BCACHE slab at the low end, adjacent to other
dynamic kernel allocations, preserving its exact MM owner,
size, contents and reclaim semantics. This is a general cache /
large-APR placement policy rather than altering a denied RUN.

The initial snapshot does not prove the candidate succeeds: a
low slab could cross another APR alignment boundary or change
memory pressure/reclaim behavior. Any production policy change
must be compiled and physically tested with 48K seven-process
admission (parent plus six children), additional packing pressure,
all restores and exact child WAIT/EXIT statuses. Existing PURE-OFF
and 32K/64K regression profiles also need validation.

P1-E remains OFF by default; strict 32K whole-KCORE optimization
stays HOLD until the normal final feature acceptance.
