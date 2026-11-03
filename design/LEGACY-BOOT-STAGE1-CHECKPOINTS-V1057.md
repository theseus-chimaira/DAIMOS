# Historical PDP-6 boot and Stage-1 checkpoints (2026-11-03, v1057)

This branch preserves eight historical 2026-10-14/15 DAIMOS branch tips
whose assembly, build and status changes predate current main:

- `origin/boot-d6lz-fixed-entry-v142-20261014`
- `origin/boot-proc-uarea-nxm-v138-20261014`
- `origin/boot-zombie-uarea-fix-v142-20261014`
- `origin/exec-transition-boot-v145-20261014`
- `origin/dtc-stage1-size-v153-20261015`
- `origin/dtc-stage1-size-v155-20261015`
- `origin/simh-go-pc-daimos-state-v155-20261015`
- local `v155-dtc-stage1-wip-20261015`

Every original tip is preserved by an ancestry-only (`ours`) merge.
Those merges retain commits and exact source content for future
forensics, but do **not** introduce old assembly into current main.

## Classification against current source

The original lowcore D6LZ36 ordering constraint remains represented
by the current KCORE_FIXED_ASM first object `d6lz_fixed`. The original
NXM and zombie-u-area changes guarded dereferences by testing the
`PROC_F_UAREA` flag; current PURE backing scans retain these guards.
The historical EXEC replacement transition code has subsequently been
restructured; it must not be transplanted into the current VM/exec ABI.

The two historical DTC Stage-1 size branches and the v155 WIP branch
contain superseded variations of the DECtape reader/decoder, not a
validated drop-in for current Stage-1. The SIMH GO-PC historical
state branch records earlier debugging; its dedicated test was
recovered and passed in testkit v1048. Do not infer a current 32K
boot result or Stage-1 word-count acceptance from this archive.

Before considering any original fix again, compare its exact commit
and caller ABI against current main and run a supported real guest
test. The user's current instruction defers further KCORE optimization
and all 32K-specific tests. Neither were performed during this pass.

Once ancestry is verified on the remote archive, the eight obsolete
branch names can be retired without losing their historical commits.
