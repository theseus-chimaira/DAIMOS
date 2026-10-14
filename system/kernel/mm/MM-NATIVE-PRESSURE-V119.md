# v119 — Native PDP-6 bounded MM pressure retries

The new `daimos-testkit/tests/native-kernel/mm/pressure-v119` suite executes
the actual production `mm_pdp6.s` on the PDP-6 simulator. Reclaim-service
entry points are independent deterministic instrumented callbacks; the MM
algorithm and retry logic are not replaced.

Tested a request exceeding all arena memory: `mm_alloc_aligned` returns
MM_ERR_NOMEM after exactly two FS reclaim callback attempts and one swap
reclaim callback attempt. No extent is inserted, the descriptor count
remains zero, and the caller's output base remains unchanged. The same
unsatisfiable request through `mm_alloc_aligned_noreclaim` returns NOMEM
without any FS or swap callbacks. Native PDP-6 test: **PASS**.

This proves the bounded failure path under callbacks that cannot reclaim
memory. It does **not** prove the success path after actual reclamation,
real swap or bcache callback behavior, fragmented/descriptor saturation
under retries, scheduler concurrency or complete operating system boot.
Those remain outstanding MM acceptance tests in KCORE.MD. The prior host
`make mm-core` suite passed in v118. No production kernel source changed.
The MM footprint stays **482 text + 70 BSS PDP-6 words**.

No BLT change was attempted: the outstanding MM pressure behavior has
priority over further instruction-level optimization. BLT is suitable only
for carefully bounded forward/downward shifts; descriptor insertion
requires backward overlap-safe movement.
