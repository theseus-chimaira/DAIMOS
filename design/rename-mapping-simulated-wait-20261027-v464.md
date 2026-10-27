# Proposal B v464: mapped RENAME under simulated provider interruption

Extended the native PDP-6 v463 integrated mapping / FILE / VFS
RENAME test into testkit v464. Its recording provider now calls a
simulated wait that overwrites volatile argument registers before
returning. The fixture saves and restores the provider opcode and
return register across that interruption. The test checks that a
single VM mapping hold remains active throughout the interruption,
that each invocation releases the hold exactly once, and that
successful and rejected operations retain the expected behavior.

Native PDP-6 test PASS. This is a controlled *simulation* of volatile
register clobbering, **not** actual `proc_wait_event` scheduling and
resumption or filesystem provider serialization. Full dispatcher and
real scheduler integration remain required for an architectural
syscall/FILE cutover. No production change. Accepted lowmem baseline
remains KCORE 8961, KCORE+MRES 15508, PERMANENT_LAST 15555.
