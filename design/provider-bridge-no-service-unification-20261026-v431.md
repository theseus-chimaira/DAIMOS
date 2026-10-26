# Proposal B v431: unify missing-service and provider unlock

Status: accepted KCORE reduction; full linked size and native PDP-6
provider lock/lease tests PASS.

The original `fs_provider_reg_call` tested for the missing-service
trampoline after taking the provider lock and duplicated the error
result before unlocking. `fs_mres_no_service` already returns -1 using
the ordinary call ABI. The serialized path can call the bound service
unconditionally, then perform the same unlock as any other result.
The lock continues to protect the entire provider call, including
potential sleeping device I/O and users of `fs_block_workspace`.

An initial fast-path experiment moved seven-AC save/restore to the
contended path but **increased linked KCORE from 8995 to 8999 words**;
it was fully reverted. A first version of the present optimization
accidentally fell through into the wait path; native simulator tests
caught the bug, and the explicit jump to unlock was restored before
acceptance. The final version preserves unlock and reduces KCORE.

Full linked GCC PDP-6 lowmem image:

                        baseline       v431       saved
KCORE                       8995        8992           3
KCORE+MRES                 15542       15539           3
PERMANENT_LAST            15589       15586           3

Existing testkit PDP-6 provider wait/retry test and deterministic
workspace interleaving test PASS. The provider-lock static contract
was adjusted for removal of the duplicate error label. The native
test also covers a missing physical service and confirms the lease is
released. No kernel stack expansion or process/u-area changes.
Strict 32K acceptance remains deferred.
