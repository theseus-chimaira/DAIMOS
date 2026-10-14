# v112 — Compact validation and gap-entry experiment (not accepted)

Baseline: v111 `mm_pdp6.s`, 482 assembled MM text words and 70 BSS.

An experiment changed `mm_compact` argument checks to use the still-live
input AC1/AC2 instead of their saved AC10/AC11 copies. This assembled to
exactly **482 words**: no code reduction, so it was discarded.

A second candidate removed the two-instruction `mm_find_fit_extent` cursor
bounds check on the theory that incoming branches already enforce its
invariant. Assembled MM text became **480 words**, BSS remained 70, and
GCC KCORE/KINIT link succeeded. But native PDP-6 `alloc-v090` **FAILED**:
the second, HIGH-preference allocation returned nonzero where it previously
succeeded (test exit 3). This demonstrates that the check cannot be removed
without a more complete control-flow/invariant proof. The change was
**reverted** rather than masking the regression. No further speed/cycle
claims or historical algorithm equivalence follow from these candidates.

Production stays unchanged at **482 text + 70 BSS words**. The next pass
should investigate `mm_find_fit` control flow with exhaustive low/high
and arena-boundary traces, or simpler proven instruction substitutions,
while preserving the original out-of-range cursor guard.
