# A04 P0-D booted progress v1200 (2026-11-05)

Partial A04/F02 acceptance evidence, not full `P0D_FULL=1` signoff.

- DAIMOS source: `08bf742781f7149e13bd10abe061026524096792`.
- Testkit: `122c61ffa81310c3fd6c038f8f7e5db2b5114e09`.
- Three individually built disposable 256K KCC PDP-6 DSK/D6FS guests.
- `daimos-session-hup-pipe-dpy-v1119.sh`: PASS. Physical PIPE and
  Type-340 DPY final HUP MM release retries, PI6 display progress,
  final WAIT and TTY ownership, zero test errors.
- `daimos-session-hup-dpy-swap-v1121.sh`: PASS. Physical DPY MM_BUSY
  across real disk swap-out/swap-in, exact swap backing release,
  retained display list and final WAIT/TTY ownership, zero errors.
- `daimos-session-hup-pipe-swap-v1122.sh`: PASS. Real first-member
  PIPE MM_BUSY then second-member disk-bitmap refusal, exact retries
  and final WAIT/TTY ownership, zero errors.

Archive: `$TMPDIR/daimos-a04-evidence-v1200.tar.gz`, SHA256
`df8d5ab7ca0721f6470cc4cdf6566437722eca6ed4d67d91aaf6c28ae97829c4`.
It contains the three raw logs and revision/log SHA256 manifest.

Still required: v1120 disabled and enabled, v1106, v1107, v1077,
v1101 plus an exact candidate complete `P0D_FULL=1` run and review.
A04/F02 remain OPEN; requester OOM remains disabled in production.
