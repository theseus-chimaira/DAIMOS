# v104 — Repeated compaction and test-fixture isolation

Real `mm_pdp6.s`, production `vm_extent_move`, and `fs_move_words` execute in
SIMH PDP-6 via `daimos-testkit/tests/native-kernel/mm/vm-live-v104`.
The extended regression repeats compaction after a successful relocation
across two arenas. It checks idempotence: result remains success, no
second PI-protected relocation occurs, the count and sorted extent base
remain unchanged. The existing 512-word payload oracle, pinned exclusion,
u-area/runqueue callbacks and negative cases remain active. **PASS**.

Expanding the fixture first caused a false regression: test symbols grew
into hard-coded synthetic process/u-area address space. The test now uses
process record at octal 06000 and u-area at 07000, outside fixture BSS.
A second unsupported assumption about fragmented placement was removed
rather than weakening production allocation semantics.

No production changes: `mm_move_extent` 66 words, `mm_compact` 62,
whole MM 493 text and 70 BSS. Multiple successful relocation steps across
many process extents, genuine runqueue/PI integration, and descriptor-table
failure injection remain open; no claim of optimality or complete stress
coverage. ITS-138/JOSS comparisons still do not supply a drop-in alternative.
