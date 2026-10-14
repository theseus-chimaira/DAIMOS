# Packed-process teardown dispatch v084 — inactive

The live assembly `vm_space_destroy` deliberately rejects PACKED storage.
Version 084 stages a dispatch function and an alternative VM destroy leaf.
RESIDENT/DISK remain under legacy cleanup. For PACKED storage, v084 first
calls the MM-authoritative registered-owner guard v39, then clears the
original executable backing record without calling `proc_swap_detach`:
that legacy detacher otherwise interprets zero-VM-base backing as an
on-disk span. The staged assembly replacement clears VM state only after
registered packed ownership has been retired.

The native testkit v084 exercises the real dispatch with deterministic
ownership-guard callback: invalid/transitionless state, RESIDENT, DISK,
PACKED discard failure (backing retained), successful PACKED discard
(backing cleared), and invalid slot rejection. PASS on PDP-6.

No production VM leaf or scheduler is changed. KINIT registry allocation,
real packed owner-discard native MM tests, and complete kernel-link/boot
validation are still required. The 8K size gate is deferred by request.
