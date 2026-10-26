# Proposal B v415: pre-cutover identity and lock contract coverage

Status: approved architecture, production encoding still unchanged.

The v414 intermediate FLOCK-only production experiment grew the full
linked image by one permanent word and was reverted. Therefore the next
production acceptance unit must replace descriptor metadata, mount
extension mapping, FLOCK ownership and ALL affected consumers together.

New regression test: `../daimos-testkit/tests/host/
daimos-file-transition-20261026-v415.c` (C99).

Host compiled with `cc -std=c99 -Os -Wall -Wextra -Werror` and passed:

    FILE v415 contract PASS: 64 mounts, PIPE/TSFS, lock, offset

Coverage: full mount-ID range with overlapping provider/local-kind;
two-word model with independent full-word offsets; separation of vnode
and metadata bitfields; provider-7 mount-zero PIPE classification versus
nonzero-mounted TSFS identity; process-local FLOCK updates affecting
same-vnode descriptors without changing per-FD permissions and APPEND;
distinct mount identities do not share lock state.

This test exercises a host-side model, not production assembly. It does
NOT establish success of RUN, FIFO reference accounting, OPEN, closed
descriptor handling, forced teardown or PDP-6 simulator execution.
Target regressions remain mandatory before accepting the cutover.

Representation-dependent production paths still include
`file_runtime.s`, `file.c`, `file.h`, `proc_run.s`, `proc_pdp6.s`,
`syscall_dispatch_pdp6.s`, `pipe.s`, and boot-initialized descriptors.
Do not partially deploy incompatible producer/consumer encodings.

Current last verified full linked baseline after v414 revert:
KCORE 9109, KCORE+MRES 15657, PERMANENT_LAST 15704 words.
No fresh production link change or RAM saving is claimed for v415.
