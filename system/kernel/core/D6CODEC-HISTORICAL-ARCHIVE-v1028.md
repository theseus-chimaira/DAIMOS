# Grouped D6CODEC cutover archive (2026-11-03 v1028)

This branch retains the original October 2026 grouped-codec cutover
history from v075, v078, v089 and v092. The extra parents were merged
with the `ours` strategy specifically to preserve their commits and
branch-specific test results, not to silently combine incompatible
kernel implementations. See each parent commit for the exact source.

Do not merge this branch directly into DAIMOS `main`. The current main
stages `d6codec_encode_v053.s`, `d6codec_decode_v053.s` and the v055
ABI wrapper without activating the grouped format. Its production boot,
EXEC, VFS, TSFS and swap paths still use D6LZ36. The archive branch's
experiments altered those paths together and cannot be independently
cherry-picked into the newer KCORE process-lifetime implementation.

The related six historical testkit branches have been consolidated at
`daimos-testkit` branch
`archive/d6codec-grouped-tests-v1027-20261103`. A current native PDP-6
v055 ABI test passed during this consolidation; the historical v091
PURE swap harness did not link against the present process and backing
store ABI. No old grouped-format full boot or 32K tests were executed.

Before a future cutover, follow the staged
`D6CODEC-INTEGRATION-TODO-v054.md` and revalidate stream geometry,
ABI register preservation, stack, all producers and all consumers
together. Never ship mixed old/new codec images.

## v1055: recover original staged-codec branch ancestry

The remote-only v053/v054 branches from 2026-10-13 are now retained
as ancestry of this experimental archive:

- `d6codec-staging-v053-20261013`
- `d6codec-integration-todo-v054-20261013`

Current DAIMOS main already carries the v053 decoder and staging note
unchanged. Its v053 encoder and v054 integration checklist have been
revised since the original branch tips. The older source and docs are
preserved for reference, not substituted for the current staged codec.
No production cutover, KCORE optimization or 32K acceptance was run.
