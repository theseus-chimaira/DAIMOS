# v122 — Native PDP-6 extent descriptor capacity boundary

The production PDP-6 `mm_pdp6.s` is tested by the versioned
`daimos-testkit/tests/native-kernel/mm/capacity-v122` fixture. It includes
the v120 bounded pressure, actual MM extent release by an instrumented
reclaim callback, and fully populated sorted descriptor exhaustion tests.

Additional verified transitions (**PASS** in PDP-6 simulator):

- From a synthetically constructed valid 32-entry table, a further
  allocation returns `MM_ERR_DESCRIPTORS`, preserving count and caller
  output, with bounded pressure callbacks under ordinary allocation.
- Reducing occupancy to 31 allows a one-word, no-reclaim allocation in
  an interior gap. Count becomes exactly 32; address is 020001 octal.
- `mm_free` removes that new descriptor and restores count to 31. No FS
  or swap callback is made in the no-reclaim/free sequence.

This validates descriptor boundary correctness and shift ordering, **not**
a measured occupancy distribution. It cannot justify a 16/24/32/48/64
boot-selected table size on its own. To choose a capacity, record actual
peak simultaneous MM descriptors during boot, D6FS/TSFS operation,
packed processes and nested MAKE/KCC/KCPP workloads. Loadable modules are
explicitly deferred to KA10/DAIMOS v1.1 and excluded from these pre-v1.1
requirements. Keep the present fixed 32-descriptor representation until
that evidence is available.

No production MM code changed: assembled text 482 words, static BSS 70
words. External reclaim callbacks remain fixtures, not the production
filesystem or process-swap implementations. No full-system native boot or
runtime resize is claimed.
