# Swap late-memory-release rollback audit v666

The production PDP-6 `proc_swap_out` assembly writes the provisional
BACKING span before trying `mm_unpin` and `mm_free`. A failure of either
MM operation branches to `proc_swap_out_record_fail` instead of the
DISK-state publication path. The rollback frees the provisional BACKING
allocation, clears the temporary u-area backing field, and clears
TRANSITION, leaving the original process extent and swap-record state
as the authoritative resident copy. No new persistent backing pointer
is published before both MM operations succeed.

The source control-flow regression in daimos-testkit v666 PASSes;
the preceding PDP-6 partial-transfer native regression v665 also
PASSes. This audit does not inject MM errors into a live process and
does not prove late rollback under all scheduler or ownership faults.
The live MM-free/unpin fault experiment and complete recovery acceptance
remain open. No kernel behavior, media format, or size optimization
was changed.
