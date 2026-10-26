# Proposal B v455: native VFS RENAME dispatch regression

The new `daimos-testkit` PDP-6 executable links production `vfs.s`
and exercises `vfs_rename` with a recording provider. It verifies
successful same-namespace operation dispatch (opcode 012), rejection
of differing provider identities and differing mount identities,
and absence of a provider call on either rejection. Native execution
PASS. The original DTFS native provider RENAME regression remains
separate; this does not constitute a full FILE-to-VFS integration test.

This step changes no DAIMOS production source, ABI, permanent state
or runtime footprint. The last fully linked image remains v452:
KCORE 8961, KCORE+MRES 15508, PERMANENT_LAST 15555.
Full FILE pathname handling, readonly mount rejection and a complete
rename transaction across provider callbacks are future acceptance
gates. Strict 32K remains deferred.
