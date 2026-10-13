# D6 fixed decoder v062 - address geometry checkpoint

The v062 candidate derives from the unchanged native v058 decoder. It adds
PDP-6 18-bit source-window and destination-output one-past-end guards and
rejects output cursors preceding the original destination base. These checks
run before the first compressed source dereference and consume no stack or
static data. The high address bound is computed in a register (`1 << 18`):
PDP-6 compare-immediate cannot represent the bound, and literal-pool loads
would not survive installation of the decoder image at fixed 000060.

The native testkit `tests/validation/d6codec-fixed-v062/run-v062.sh` verifies
invalid source and destination ranges, and an invalid output base. All passed
on PDP-6 after correction of the test fixture's AC14 initialization. Existing
v058 fixed-address, incomplete-group, malformed stream, and VFS tests passed
against v058, which is intentionally preserved without modification.

Standalone v062 low-core image: 139 decimal 36-bit words, zero BSS and zero
stack use. Legacy core is 40 words, so the fixed code increase is 99 words.
The new v062 geometry checks improve correctness but do not meet the final
no-extra-resident-RAM constraint. Full-KCORE comparison and VFS/Stage-1
runtime integration remain pending. This is an experimental checkpoint,
not a production boot-format switch.
