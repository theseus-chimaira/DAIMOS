# K05 implementation closure v1916

Date: 2026-11-24. Base: DAIMOS 639aecca (v1915).

## Defect and correction

The optional PDP-6 RAW PURE service implements the permanent
`proc_swap_reclaim` trampoline. Both callers (mm_alloc_aligned
and vm_brk_fallback) explicitly stop on typed hard MM errors
below MM_ERR_DESCRIPTORS. The old service instead collapsed
mm_compact failures to ordinary memory pressure, potentially
reclaiming sleeping process prefixes after a failed MM move.

v1916 propagates the exact hard result before and after DROP.
The two post-DROP compaction paths share one call site. A safely
refused DROP with an exact unpinned original resident owner
can still be skipped; a committed exact PACKED owner remains
committed. If neither ownership proof succeeds, leave TRANSITION
held and stop pressure with the original hard error (or BUSY).
There is no new owner state, no forced release and no ABI change.

## Focused validation

The existing testkit pure-service-v1612 native runner builds the
actual production service, DROP, prefix and FULL source with KCC,
and links actual PDP-6 MM rebase/expand and storage-mode assembly.
The compactor return and invalid descriptor are controlled test
fixtures, not organic physical allocator failures.

- Original v1915: new test fails at native exit 0601 octal.
- Corrected v1916: initial and post-DROP INVAL/BUSY preserve
  exact return and resident/PACKED ownership; corrupt DROP size
  remains unchanged, with TRANSITION held and no additional
  compaction. All five new cases pass.
- Existing DROP/prefix/FULL and refusal cases pass; existing
  unsafe READY_ONCE negative control rejects.

Native linked service bounds: reclaim entry 005714 in both builds;
next module entry mm_pure_rebase_pinned moves 006271 to 006301,
an increase of 010 octal (8 decimal) words in this standalone
linked fixture. Fixed KCORE and persistent data source unchanged.
This is not a measured integrated MRES image delta.

## Closure boundary

Earlier v1808-v1915 evidence covers lost replies, source failure
and recovery, exact one-pin commit, physical refusal and PI6,
deferred rollback, parent/last-user exit, timer wrap and
requester-only capacity OOM. v1855 covers repeated FULL allocation
NOMEM with original RAW survival. v1916 closes the remaining
identified reclaim error-propagation defect. No new exhaustive
physical test sweep is required for implementation closure.

K06 owns persistent/ambiguous owner and pin reconciliation;
K07 owns backing lifetime and source validity; K08 owns scheduling
and progress; K09 owns current-source production integration.
Organic fragmentation/descriptor exhaustion and other physical
allocator/PI6 combinations, root/platform matrices and final
size/boot acceptance are not proven by v1916 and remain release
gates. Neither RAW PURE nor mode3 is enabled by default.
