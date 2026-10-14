# v100 — Real production PDP-6 VM relocation regression

The standalone native test in `daimos-testkit/tests/native-kernel/mm/vm-live-v100`
executes production `mm_pdp6.s` together with the **exact assembler bodies**
of `vm_extent_move` from `vm_pdp6.s` and `fs_copy_words`/`fs_move_words`
from `fs_mres.s`. The test-run script extracts these bodies at build time;
no equivalent reimplementation of MM, VM, or the memory-copy algorithm is
used. External process-runqueue, process-table initialization and PI hooks
remain deterministic fixtures; the complete kernel is not booted.

PDP-6 simulator regression **PASS**:

- A sleeping, non-current process relocates upward, with MM descriptor and
  process VM base updated, first/last physical words preserved, and PI
  disable/restore counts matching.
- Relocating the current process returns BUSY without modifying memory.
- Relocating a process marked TRANSITION returns BUSY without data movement.

This strengthens the v099 callback-only MM test with actual VM and BLT
execution. It does **not** establish interrupt concurrency, translated
mapping/UAREA handling, full payload integrity, or boot-system safety.
The underlying relocation helper remains **66 words** and the complete MM
assembler remains **493 text + 70 BSS words**. No production algorithm was
changed. Next tests should cover pinned ranges, u-area mapping holds,
runqueue restoration, overlapping moves, full-image checksums and failure
injection before shrinking descriptor publication or PI-critical sections.

ITS-138 and JOSS compaction remain historical design references; neither is
a directly comparable drop-in algorithm with established code savings.
