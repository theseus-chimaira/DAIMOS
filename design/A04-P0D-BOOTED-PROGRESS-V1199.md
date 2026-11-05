# A04 P0-D full-guest progress v1199 (2026-11-05)

This is an additional subset of the booted `P0D_FULL=1` acceptance
matrix, **not** final A04/F02 signoff.

## Source/testkit provenance

- DAIMOS: `3eb2d0b32ffb9870bcb497c651238c469f23eca5`.
- daimos-testkit: `122c61ffa81310c3fd6c038f8f7e5db2b5114e09`.
- Platform: Android/Termux, installed `$HOME/cross` PDP-6 toolchain
  and simulator, disposable 256K KCC DSK/D6FS guest images.
- Each test executed individually with `DAIMOS_REPO=$HOME/git/DAIMOS`,
  `PDP10_PREFIX=$HOME/cross`, `$PDP10_PREFIX/bin` on PATH and
  `TMPDIR=${TMPDIR:-$HOME/tmp}`.

## Results

- `daimos-oom-wait-churn-v1117.sh`: **PASS**. Mixed fatal teardown,
  48 stale WAIT/PID/event refusals, no process MM/slot growth, live
  bystander and exact resource owner release.
- `daimos-oom-physical-mixed-v1118.sh`: **PASS**. Actual PIPE and
  process VM MM_BUSY in one faulted process, physical MM release,
  one swap detach, foreign provider ownership, final WAIT and zero
  reported errors. Retains v1117 48-stale-request checks.
- `daimos-session-hup-pipe-mm-v1104.sh`: **PASS**. Physical session
  HUP with pinned PIPE final close and member u-area; one MM_BUSY,
  one unpin, one successful release, exact TTY handoff, errors zero.
- `daimos-session-hup-dpy-mm-v1105.sh`: **PASS**. Physical HUP with
  pinned Type-340 DPY raw-list MM_BUSY and PI6 replay, deferred
  u-area/WAIT; 14 DPY refresh observations, eight after busy,
  one eventual free, errors zero.

Raw logs and checksum manifest are archived in
`$TMPDIR/daimos-a04-evidence-v1199.tar.gz`.
Archive SHA256:
`4db2b9553f08fbc84fed8e6b5c019512db172f806dee46d30e256cad2809277e`.

The preceding v1197/v1198 reports contain earlier component,
v1125, v1124, v997 and v1103 evidence. These new runs do not
constitute a fresh uninterrupted full matrix. Next unexecuted row
in matrix order is v1119 (PIPE + DPY final HUP); complete remaining
v1119/v1121/v1122/v1120 off+on/v1106/v1107/v1077/v1101 guests,
then rerun an exact-revision final full matrix with build/owner
counters and release diagnostics. A04 and F02 remain OPEN;
production OOM default is OFF; no kernel/ABI/footprint changes.
