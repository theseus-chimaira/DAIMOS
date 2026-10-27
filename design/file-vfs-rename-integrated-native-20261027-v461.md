# Proposal B v461: native FILE-to-VFS RENAME integration

The v460 cutover gate required testing across existing resident layers
before removing ownership wrappers. The new testkit v461 PDP-6 native
oracle links the real `file_rename` frame and real resident `vfs.s`
against a recording filesystem provider. It verifies successful
RENAME dispatch opcode 012, the two parent-name resolutions, the two
permission checks, failures at either resolution or permission step,
and provider failure propagation. All cases PASS under native PDP-6
instruction execution.

This removes one important fixture coverage gap but does not exercise
the native syscall mapping veneer, a sleeping filesystem provider, or
the full mapped-user-pointer lease. Those remain mandatory before a
coordinated syscall/FILE wrapper removal. No production change in
v461; accepted linked KCORE stays 8961, KCORE+MRES 15508 and
PERMANENT_LAST 15555. Strict 32K acceptance remains deferred.
