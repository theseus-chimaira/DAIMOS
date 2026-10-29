# Native PDP-6 VOLUME mapper-loss test v664

The v659 fail-closed behavior is now executed under native PDP-6
instruction emulation in daimos-testkit v664. The fixture extracts the
actual production BACKING transfer assembly from `fs_mres.s` and links
it with instrumented DSK270/DRM236 device entry points.

The test configures a mapped VOLUME without an installed mapper and
verifies `backstore_read` returns -1 without invoking device I/O.
It then configures a direct DEVICE region and verifies that a read
still reaches the physical DSK path and succeeds. The test passes.

This closes isolated native missing-mapper coverage, but does not
simulate a mapper disappearing in a running full-system swap or a
controller failure partway through a multi-block transfer.
Production kernel code and media formats are unchanged. No
optimization work was performed.
