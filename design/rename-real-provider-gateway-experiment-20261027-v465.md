# Proposal B v465: real FS_MRES gateway integration experiment

Attempted to extend the v464 PDP-6 mapped RENAME oracle by extracting
the real `fs_provider_reg_call` / physical-provider serialization
gateway from `fs_mres.s`, binding a recording DTFS service, and
forcing one wait/retry by beginning with the provider lock unavailable.

The combined fixture linked after removing a duplicate test-defined
`kconst_7_7`, but the first successful RENAME assertion failed in the
native simulator (`__test_exit=1`). The root cause has **not** been
established. It would be incorrect to call this a verified provider
sleep/wakeup regression or accept an architectural cutover on it.

The exploratory testkit v465 files were discarded; existing native
v463/v464 passing regressions remain intact. No production kernel
source was modified, and the accepted linked lowmem measurements
remain KCORE 8961, KCORE+MRES 15508, PERMANENT_LAST 15555.

Next: isolate gateway service binding, lock-state transitions, and
register restoration in the combined fixture, distinguish a test
fixture error from a real kernel error, and rerun a native combined
integration check. Actual scheduler context switching is still not
covered by a simple wait callback.
