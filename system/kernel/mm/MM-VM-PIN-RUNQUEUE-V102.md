# v102 — Native PDP-6 pin exclusion and VM runqueue preservation

The `daimos-testkit/tests/native-kernel/mm/vm-live-v102` suite executes the
production `mm_pdp6.s`, the exact extracted `vm_extent_move` body from
`vm_pdp6.s`, and production `fs_move_words`/`fs_copy_words` instructions
on the standalone PDP-6 simulator. External PI and runqueue entry points
remain instrumented test callbacks rather than the live kernel scheduler.

New v102 tests PASS: an eligible process with a stable u-area is stopped
for relocation and its runqueue remove/add callbacks both execute exactly
once; the process's VM relocation base updates and the dedicated MM stop
reason is cleared afterwards. A process extent with pin count one is
excluded from compaction and leaves its MM descriptor unchanged, with
no PI disable/restore call. The test continues to verify the full
01000-octal-word copy and current/transition/mapping/unaligned rejection
cases from v101.

There was an initial invalid test fixture (misencoded pinned descriptor),
corrected to the actual `MM_PIN_SHIFT=21`/type packing. The corrected
fixture passes. No production MM or VM instruction changed.

Size baseline is unchanged: `mm_move_extent` **66 PDP-6 words**,
`mm_compact` **62**, total MM text **493**, static BSS **70**.

Remaining gates before rearranging relocation: full real scheduler queue
integration, interrupt timing, repeated compaction across multiple arenas,
failed descriptor publication and overlapping copies. ITS-138 and JOSS
remain architectural references rather than drop-in replacements.
