# Staged PDP-6 packed-process lifecycle gate v082

The inactive `proc_packed_lifecycle_v082.c` checks the existing process
state and storage discriminator before invoking the v079 exact-size inline
codec eviction/restoration API. An eviction candidate must not be the
running process, must be RUN/SLEEP/STOP with u-area and executable backing,
without transition or user mapping hold, and must own a resident VM extent.
Restoration requires a nonresident runnable PACKED process, no transition,
and a PURE reloader. Transition is acquired before calling the native
transaction and cleared on precommit failures; the restore postcommit
failure retains TRANSITION when its resident base was already published.

The v082 testkit compiles the real gate and the v077/v46/v079 transactions
for PDP-6, links with the v078 encoder and v067 decoder, and runs all four
combinations of dense/sparse registry and zero/mixed payloads. It checks
rejection of the current process and already-transitioning processes,
allocation rollback, subsequent packed eviction and exact restoration.
All PASS in SIMH with deterministic MM allocations.

The active PDP-6 scheduler's swapping and slot-0 service are assembly in
`proc_swap_pdp6.s`; this staged gate is NOT linked there yet. A storage-mode
aware dispatcher is required before changing scheduler routing: DISK must
remain distinct from PACKED. KINIT registry allocation, lifecycle teardown,
real MM arenas and full boot remain outstanding. The 8K size gate is
explicitly deferred for this implementation phase.
