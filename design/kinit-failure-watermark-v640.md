# KINIT fatal-path stack measurement v640

The optional PDP-6 KINIT watermark previously sampled the stack during
successful initialization but not when an early error terminated through
`kinit_halt` or `kinit_error18`. In watermark-enabled builds only, both
halt entries now record the current highest touched stack offset into
the existing `kinit_stack_highwater` word before executing HALT.
Production (`KINIT_STACK_WATERMARK=0`) code has no additional instructions,
word allocations or control-flow changes.

Native verification reused the separate 48K GCC lowmem tape-root image
with watermarking enabled and injected console root selector `7`, which
cannot match a supported root class. SIMH reported an actual HALT and
`kinit_stack_highwater` at octal `0164` (116 decimal) words, safely below
the octal `0600` (384 decimal) reserve. Testkit
`daimos-kinit-failure-watermark-v640.sh` rebuilds the instrumented image,
forces the invalid-root fault, reads the recorded word after the HALT,
and rejects a missing, zero or out-of-range measurement.

This proves one native fatal root-selection path, not all KINIT failure
paths, a complete 32K failure-path watermark proof, or 32K kernel-only
acceptance. Those remain open tasks.
