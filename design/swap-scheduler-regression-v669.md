# Full-system swap scheduler acceptance investigation v669

The historical testkit v298 full-system regression does not run against
the current split KCORE/MRES layout: `proc_swap_out` and `proc_swap_in`
are loadable SWAP MRES entry points, absent from `kcore.map` and present
as relative offsets in `swap-mres-v481.map`. v298 exits during symbol
resolution before launching SIMH, without a diagnostic message.

Testkit v669 removes invalid fixed-address swap-entry breakpoints and
offers optional resident KCORE tracing and reuse of a separately built
full boot image. With tracing enabled, SIMH reached the workload but
timed out after 300 seconds of repeated breakpoint activity. With
tracing disabled, SIMH boots the controller/MRES inventory and then
prints `?RT`, halting at `065206`, without the pressure workload's
expected completion marks. v669 therefore **does not pass**.

The isolated native tests v664-v668 remain passing evidence for the
BACKING mapper, partial-write and late MM rollback branches. They do
not establish full-system scheduler recovery. Next investigate the
`?RT` startup halt and trace the actual loaded SWAP MRES calls without
fixed offsets or high-volume breakpoints. No kernel change or size
optimization was made in v669.
