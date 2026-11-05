# A04 P0-D final individual guest row coverage v1201 (2026-11-05)

This report records *individually executed* booted acceptance rows.
It does not certify a single uninterrupted exact-revision
`P0D_FULL=1` matrix, final A04 integration, or F02 release signoff.

## Frozen provenance

- DAIMOS `d9aedf20193b3113430eae70a81680911ab25145`.
- daimos-testkit `122c61ffa81310c3fd6c038f8f7e5db2b5114e09`.
- Disposable 256K PDP-6 full KCC DSK/D6FS guests, installed
  `$PDP10_PREFIX=$HOME/cross` simulator and toolchain.

## Results

- v1120 production-OOM-disabled: PASS. Genuine BRK physical OOM
  returns -1 without killing requester. 24 numeric PID reuse
  lifetimes, alternating EXIT, stale WAIT/TERM refusal and exact
  process/MM/slot cleanup.
- v1120 isolated-OOM-enabled: PASS. Physical shortage terminates
  only requester, witness runs, MM accounting recovers; same
  24 PID reuse and stale event checks.
- v1106: PASS. Physical process-VM pin/MM_BUSY, true MM free and
  swap detach, u-area retry, held foreign filesystem-provider lease,
  CHLD/WAIT and independent bystander.
- v1107: PASS. Real DISK swap-out and physical bitmap verifier
  refusal, exact bitmap repair, retained swap record/WAIT, final
  freeing and HUP.
- v1077: PASS. Actual D6FS SYNC (two refusals) and MM_BUSY (one
  refusal) during physical requester OOM, witness and recovery.
- v1101: PASS. Persistent physical MM wrong-owner -4, seven
  invalid release refusals, capped idle reaper, preserved owner,
  unreaped WAIT and independent bystander.

Evidence archive: `$TMPDIR/daimos-a04-evidence-v1201.tar.gz`.
SHA256: `7234ecd1061d6f410ebdbf89476c19b2b346d1709532d2d46219755c93e9c16c`.
Archive includes six full logs and an exact source/testkit provenance
and per-log SHA256 manifest.

## Acceptance scope still open

Across v1197-v1201 all 17 booted guest invocations in the current
v1096 P0D_FULL script have been run individually and reported PASS,
and the 18 native/source-linked component stages and v1083/v1087
audits PASS in earlier preflight. The evidence spans commits
`d3e02fdc`, `22df1a0c`, `3eb2d0b3`, `08bf7427`, and `d9aedf20`,
which differ only by documentation within this testing sequence.
Nevertheless **no single uninterrupted final candidate full matrix**
has been completed, and no final exact-revision resident-size,
diagnostic and ownership review has been signed off.

A04 and F02 remain OPEN. Production requester-OOM switch is OFF.
The next task is a complete frozen full `P0D_FULL=1` matrix and
formal exact-candidate acceptance review; do not extrapolate from
these staggered runs to declare the release gates closed.
