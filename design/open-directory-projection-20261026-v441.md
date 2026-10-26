# Proposal B v441: compact OPEN directory type projection

Accepted small KCORE reduction. `file_open_newfd` receives the already
validated VFS type in the seven-word stat record. Directory type is
exactly one. Load that value into the descriptor directory flag argument
and clear it for every other type; this replaces four instructions with
three, retaining the original zero/one API for `file_new_fd`.

Full linked GCC PDP-6 lowmem KCORE **8987**, from 8988 (one word saved);
KCORE+MRES 15534; PERMANENT_LAST 15581. No stack or permanent storage
growth. Testkit FILE direct-mount, PIPE direct-mount, UMASK and static
OPEN lifetime regression PASS. New exhaustive 64-type projection guard
PASS. KCC DSH size gate remains unchanged and is separately satisfied
by the accepted v440 source fix. Strict 32K acceptance deferred.

This is not completion of the planned FILE/VFS/syscall consolidation.
