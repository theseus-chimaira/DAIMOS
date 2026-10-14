# v101: Full-payload native PDP-6 relocation verification

The daimos-testkit standalone `tests/native-kernel/mm/vm-live-v101`
regression executes production `mm_pdp6.s`, the extracted exact production
`vm_extent_move`, and the exact production `fs_move_words`/`fs_copy_words`.
Only external process/PI hooks are deterministic test fixtures.

**PDP-6 simulator PASS:** all 01000 octal (512 decimal) relocated words are
verified against a per-address XOR pattern, rather than testing only the
endpoints. The suite also verifies original MM/process VM relocation bases,
PI disable/restore, current-process rejection, transition-state rejection,
unaligned destination rejection, and translated user-mapping hold rejection.
Rejection is checked without data mutation at the observed source word.

This does **not** yet exercise actual scheduler queues or asynchronous PI,
full u-area stop/resume, repeated overlapping relocations, or long-running
randomized compaction. No production MM code changed. `mm_move_extent`
remains 66 words; `mm_compact` 62; total MM `.text` 493 and `.bss` 70.
Do not optimize descriptor transaction ordering before verifying the remaining
state and rollback paths. The ITS-138 and JOSS comparisons remain technique
references, not drop-in allocator replacements.
