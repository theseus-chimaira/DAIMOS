# Proposal B v428: native workspace lease interleaving baseline

Status: new PDP-6 deterministic interleaving test PASS; no kernel code
change and no memory saving claimed.

Added `daimos-testkit/tests/validation/daimos-provider-workspace-interleave-v428.{s,sh}`.
It executes the real resident `fs_provider_reg_call` assembly. Its
simulated `proc_wait_event` observes a blocked request, checks that the
shared 128-word workspace remains unchanged, releases the lease, and
clobbers the request ACs. The provider must only then dispatch with all
original arguments and return the expected result. Native PDP-6 PASS.

This tests a deterministic scheduling interleaving, not two independent
processes doing real disk I/O. It does not establish a safe new lock
implementation. The v427 source analysis found that TSFS table readers
return *pointers* to records inside the shared transfer buffer. Such
consumers cannot release the buffer immediately after I/O. Replacing
the global lock must either transfer record data into separately owned
storage or change the provider API so consumers complete while holding
the lease. Independent in-flight full-block buffers would cost at least
128 physical words each, which must be included in the RAM comparison.

Next: identify individual pointer-returning operations and assess
whether a value-returning, bounded record API reduces the lease scope
without more permanent code or materially larger kernel stacks. A full
concurrent I/O simulator test is required before changing serialization.

Current accepted full image remains KCORE 8995, KCORE+MRES 15543,
PERMANENT_LAST 15590. Strict 32K acceptance remains deferred.
