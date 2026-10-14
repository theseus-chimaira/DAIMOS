# Staged slot-0 packed/disk restore routing v083

`proc_swap_restore_dispatch_v083()` consults the two-bit per-process
storage discriminator before choosing a restore implementation. DISK goes
to the unchanged `proc_swap_in`; PACKED requires nonzero original backing
and calls the lifecycle-gated v082 inline restore with the production
`proc_swap_reload_pure`. RESIDENT and invalid modes are rejected.

The inactive `proc_swap_pdp6_inline_v083.s` is a separate variant of the
PDP-6 swap assembly with the slot-0 restore call redirected through this
dispatcher. The active `proc_swap_pdp6.s` is not modified.

Native PDP-6 testkit v083 verifies dispatch to DISK versus PACKED, rejects
missing backing/invalid slot/RESIDENT, and checks assembly call-site text.
PASS. The alternate assembly has not yet been full-kernel linked, and the
scheduler/boot integration remains inactive. A standalone DAS attempt
failed on missing normal kernel symbols; no standalone assembly-link success
is claimed. Kernel-link, MM lifecycle teardown, KINIT registry allocation,
and full simulator boot are still required. Resident size work is deferred.
